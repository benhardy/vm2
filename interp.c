#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "opcodes.h"

#define TOKEN_NUMBER        0x01

#define TOKEN_PLUS          0x12
#define TOKEN_MINUS         0x13

#define TOKEN_MULT          0x34
#define TOKEN_DIV           0x35
#define TOKEN_MOD           0x36

#define TOKEN_OPEN_PAREN    0x80
#define TOKEN_CLOSE_PAREN   0x81

#define TOKEN_TABLE_SIZE 0x1000

#define INPUT_SIZE_MAX 0x10000

typedef struct _Token {
  int kind;
  int token_table_index;
  struct _Token *next; // singly linked list.
} Token;

char token_table[TOKEN_TABLE_SIZE];

Token * new_token(int kind, int token_table_index, Token*next) {
  Token *token = (Token*) malloc(sizeof(Token));
  token->kind = kind;
  token->token_table_index = token_table_index;;
  token->next = NULL;
  return token;
}

void free_token_list(Token*head) {
  while(head) {
    Token * next = head->next;
    free(head);
    head = next;
  }
}

char* token_to_string(Token*token) {
  if (token->kind== TOKEN_NUMBER && token->token_table_index >= 0)
    return &token_table[token->token_table_index];
  switch(token->kind) {
    case TOKEN_PLUS: return "+";
    case TOKEN_MINUS: return "-";
    case TOKEN_MULT: return "*";
    case TOKEN_DIV: return "/";
    case TOKEN_MOD: return "%";
    case TOKEN_OPEN_PAREN: return "(";
    case TOKEN_CLOSE_PAREN: return ")";
  }
  return "UNKNOWN";
}

char input[INPUT_SIZE_MAX];

typedef struct _Buffer {
  int size;
  char *content;
} Buffer;

Buffer * new_buffer(int size) {
  Buffer * buf = (Buffer*) malloc(sizeof(Buffer) + size);
  buf->size = size;
  buf->content = (char*) (((int*) buf) + 1);
  return buf;
}

void delete_buffer(Buffer* buffer) {
  free(buffer);
}


Token* scan_input(Buffer *buffer) {
  Token *last = NULL;
  Token *head = NULL;
  char ch;
  int buf_pos = 0;
  int token_table_index = 0;
  int kind;
  while((ch=(buffer->content[buf_pos++]))) {
    if (isspace(ch))
      continue;
    int token_start = token_table_index;
    Token * current_token = NULL;
    if (isdigit(ch)) {
      do {
        token_table[token_table_index++] = ch;
        ch = buffer->content[buf_pos++];
      } while(isdigit(ch));
      buf_pos--; // spit that last character back out
      token_table[token_table_index++] = '\0';
      current_token = new_token(TOKEN_NUMBER, token_start, NULL);
    }
    else {
      switch(ch) {
        case '(': kind = TOKEN_OPEN_PAREN; break;
        case ')': kind = TOKEN_CLOSE_PAREN; break;
        case '+': kind = TOKEN_PLUS; break;
        case '-': kind = TOKEN_MINUS; break;
        case '*': kind = TOKEN_MULT; break;
        case '/': kind = TOKEN_DIV; break;
        case '%': kind = TOKEN_MOD; break;
        default:  kind = 0;
      }
      if (kind) {
        current_token = new_token(kind, -1, NULL);
      }
    }
    if (current_token == NULL) {
      printf("Invalid token found at position %d starting with '%c'\n", buf_pos, ch);
      return NULL;
    }
    if (last) {
      last->next = current_token;
    }
    if (!head) {
      head = current_token;
    }
    last = current_token;
  }
  return head;
}

typedef struct _AST_Node {
  bool leaf;
  Token*token;
  struct _AST_Node *left;
  Token* operator;
  struct _AST_Node *right;
} AST_Node;

AST_Node * new_leaf_node(Token*token) {
  AST_Node *node = (AST_Node*) malloc(sizeof(AST_Node));
  node->leaf = true;
  node->token = token;
  node->left = NULL;
  node->right = NULL;
  node->operator = NULL;
  return node;
}
AST_Node * new_branch_node() {
  AST_Node *node = (AST_Node*) malloc(sizeof(AST_Node));
  node->leaf = false;
  node->token = NULL;
  node->left = NULL;
  node->right = NULL;
  node->operator = NULL;
  return node;
}

AST_Node * parse_addables(Token** tokens);
AST_Node * parse_multipliables(Token** tokens);
AST_Node * parse_term(Token** tokens);

AST_Node * parse_term(Token** tokens) {
  printf("parse_term starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  //printf("parse_term starts B, current=%08lx\n", current);
  AST_Node * node = NULL;
  //printf("parse_term starts C, node=%08lx\n", node);
  if (current->kind == TOKEN_OPEN_PAREN) {
    puts("parse_term found a paren, calling parse to extract subexpression");
    node = new_branch_node();
    current = current->next;
    node->left = parse_addables(&current);
    if (!current || current->kind != TOKEN_CLOSE_PAREN) {
      fputs("closing parenthesis expected\n", stderr);
      return NULL;
    }
    current = current->next;
    *tokens = current;
    return node;
  }
  if (current->kind == TOKEN_NUMBER) {
    puts("parse_term found a number\n");
    node = new_leaf_node(current);
    current = current->next;
    *tokens = current; // we consume this token, update pointer
    return node;
  }
  fprintf(stderr, "Term expected at %s\n", token_to_string(current));
  return NULL;
}

AST_Node * parse_multipliables(Token** tokens) {
  printf("parse starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  AST_Node * lhs = parse_term(&current);
  if (!lhs) {
    return NULL;
  }
  while (current && (current->kind == TOKEN_MULT || current->kind == TOKEN_DIV || current->kind == TOKEN_MOD)) {
    printf("parse: found connecting operator %s\n", token_to_string(current));
    Token * operator = current;
    current = current->next;
    AST_Node * rhs = parse_term(&current);
    if (!rhs) {
      fputs("right hand side expected\n", stderr);
      return NULL;
    }
    AST_Node *parent = new_branch_node();
    parent->left = lhs;
    parent->right = rhs;
    parent->operator = operator;
    lhs = parent;
  }
  *tokens=current;
  return lhs;
}


AST_Node * parse_addables(Token** tokens) {
  printf("parse starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  AST_Node * lhs = parse_multipliables(&current);
  if (!lhs) {
    return NULL;
  }
  while (current && (current->kind == TOKEN_PLUS || current->kind == TOKEN_MINUS)) {
    printf("parse: found connecting operator %s\n", token_to_string(current));
    Token * operator = current;
    current = current->next;
    AST_Node * rhs = parse_multipliables(&current);
    if (!rhs) {
      fputs("right hand side expected\n", stderr);
      return NULL;
    }
    AST_Node *parent = new_branch_node();
    parent->left = lhs;
    parent->right = rhs;
    parent->operator = operator;
    lhs = parent;
  }
  *tokens=current;
  return lhs;
}

AST_Node* begin_parsing(Token*head) {
  return parse_addables(&head);
}

void free_tree(AST_Node* root) {
  fputs("free_tree not implemented\n", stderr);
}

void print_postfix(AST_Node* tree) {
  if (tree->leaf) {
    fputs(token_to_string(tree->token), stdout);
  } else {
    if (tree->right) {
      fputs("[", stdout);
      print_postfix(tree->left);
      fputs(" ", stdout);
      print_postfix(tree->right);
      fputs("]", stdout);
      fputs(token_to_string(tree->operator), stdout);
    } else {
      print_postfix(tree->left);
    }
  }
}
  

int main(int argc, char**args) {
  bool keep_going = true;
  Buffer buf;
  buf.size = INPUT_SIZE_MAX;
  buf.content = input;
  while(keep_going) { 
    Token * token_list = NULL;
    Token *cur;
    printf("vm> ");
    if (!fgets(buf.content, buf.size, stdin)) 
      break;
    token_list = scan_input(&buf);
    if (!token_list) {
      continue;;
    }
    cur = token_list;
    while(cur) {
      printf("TOKEN kind=%d value=%s\n", cur->kind, token_to_string(cur));
      cur = cur->next;
    }
    puts("caling parse\n");
    AST_Node *root = begin_parsing(token_list);
    if (!root) {
      fputs("Syntax error", stderr);
    } else {
      puts("caling print_postfix\n");
      print_postfix(root);
      puts("\n done caling print_postfix\n");
      free_tree(root);
    }
    free_token_list(token_list);
  }
}

