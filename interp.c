#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "opcodes.h"
#include "vm.h"

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

//#define TRACE_ON

#ifdef TRACE_ON
#define log_trace(...) fprintf(stderr,  __VA_ARGS__)
#else
#define log_trace(...) ;
#endif
#define log_warn(...) fprintf(stderr,  __VA_ARGS__)

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
  bool apply_unary_minus; // result must be negated
} AST_Node;

AST_Node * new_leaf_node(Token*token) {
  AST_Node *node = (AST_Node*) malloc(sizeof(AST_Node));
  node->leaf = true;
  node->token = token;
  node->left = NULL;
  node->right = NULL;
  node->operator = NULL;
  node->apply_unary_minus = false;
  return node;
}
AST_Node * new_branch_node() {
  AST_Node *node = (AST_Node*) malloc(sizeof(AST_Node));
  node->leaf = false;
  node->token = NULL;
  node->left = NULL;
  node->right = NULL;
  node->operator = NULL;
  node->apply_unary_minus = false;
  return node;
}

AST_Node * parse_addables(Token** tokens);
AST_Node * parse_multipliables(Token** tokens);
AST_Node * parse_term(Token** tokens);
AST_Node * parse_unary_minus(Token** tokens);

void write_instructions(AST_Node* tree, int32_t*code);

AST_Node * parse_term(Token** tokens) {
  log_trace("parse_term starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  //printf("parse_term starts B, current=%08lx\n", current);
  AST_Node * node = NULL;
  //printf("parse_term starts C, node=%08lx\n", node);
  if (current->kind == TOKEN_OPEN_PAREN) {
    log_trace("parse_term found a paren, calling parse to extract subexpression");
    current = current->next;
    AST_Node * child = parse_addables(&current);
    if (!current || current->kind != TOKEN_CLOSE_PAREN) {
      fputs("closing parenthesis expected\n", stderr);
      return NULL;
    }
    current = current->next;
    *tokens = current;
    if (child->leaf || !child->right)
      return child;
    node = new_branch_node();
    node->left = child;
    return node;
  }
  if (current->kind == TOKEN_NUMBER) {
    log_trace("parse_term found a number %s\n", token_to_string(current));
    node = new_leaf_node(current);
    current = current->next;
    *tokens = current; // we consume this token, update pointer
    return node;
  }
  fprintf(stderr, "Term expected at %s\n", token_to_string(current));
  return NULL;
}

AST_Node * parse_unary_minus(Token** tokens) {
  log_trace("parse_unary_minus starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  if (!current)
    return NULL;
  if (current->kind != TOKEN_MINUS) {
    return parse_term(tokens);
  }
  current = current->next;
  if (!current) {
    fputs("expression expected after unary minus", stderr);
    return NULL;
  }
  AST_Node *term = parse_term(&current);
  if (!term) {
    fputs("unary - present but no term after\n", stderr);
    return NULL;
  }
  *tokens = current;
  log_trace("processing unary minus. term unaryism was %d\n", term->apply_unary_minus);
  term->apply_unary_minus = ! term->apply_unary_minus;
  log_trace("processing unary minus. term unaryism is now %d\n", term->apply_unary_minus);
  return term;
}

AST_Node * parse_multipliables(Token** tokens) {
  log_trace("parse starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  AST_Node * lhs = parse_unary_minus(&current);
  if (!lhs) {
    return NULL;
  }
  while (current && (current->kind == TOKEN_MULT || current->kind == TOKEN_DIV || current->kind == TOKEN_MOD)) {
    log_trace("parse: found connecting operator %s\n", token_to_string(current));
    Token * operator = current;
    current = current->next;
    AST_Node * rhs = parse_unary_minus(&current);
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
  log_trace("parse starts &tokens=%08lx\n", tokens);
  Token *current = *tokens;
  AST_Node * lhs = parse_multipliables(&current);
  if (!lhs) {
    return NULL;
  }
  while (current && (current->kind == TOKEN_PLUS || current->kind == TOKEN_MINUS)) {
    log_trace("parse: found connecting operator %s\n", token_to_string(current));
    Token * operator = current;
    current = current->next;
    AST_Node * rhs = parse_multipliables(&current);
    if (!rhs) {
      log_trace("right hand side expected\n");
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
  log_warn("free_tree not implemented\n");
}

void print_postfix(AST_Node* tree) {
  if (tree->leaf) {
    if (tree->apply_unary_minus)
      fputs("-", stdout);
    fputs(token_to_string(tree->token), stdout);
  } else {
      fputs("[", stdout);
      print_postfix(tree->left);
      if (tree->right) {
        fputs(",", stdout);
        print_postfix(tree->right);
      }
      fputs("]", stdout);
      if (tree->right) {
        fputs(token_to_string(tree->operator), stdout);
      }
    if (tree->apply_unary_minus)
      fputs("neg", stdout);
  }
}


void write_instructions_rec(AST_Node* tree, int32_t*code, int* code_pos) {
  if (tree->leaf) {
    int value = atoi(token_to_string(tree->token));
    if (tree->apply_unary_minus)
      value = -value;
    fprintf(stdout, "%04x %10s[%02x] #0x%04x\n", *code_pos, "PUSH", I_PUSH, value);
    code[(*code_pos)++] = I_PUSH;
    code[(*code_pos)++] = value;
  } else {
    write_instructions_rec(tree->left, code, code_pos);
    if (tree->right) {
      write_instructions_rec(tree->right, code, code_pos);
      int op =0;
      switch(tree->operator->kind) {
        case TOKEN_MULT: op = I_MUL; break;
        case TOKEN_DIV: op = I_DIV; break;
        case TOKEN_MOD: op = I_MOD; break;
        case TOKEN_PLUS: op = I_ADD; break;
        case TOKEN_MINUS: op = I_SUB; break;
      }
      fprintf(stdout, "%04x %10s[%02x] \n", *code_pos, instructions[op], op);
      code[(*code_pos)++] = op;
    }
    if (tree->apply_unary_minus) {
      fprintf(stdout, "%04x %10s[%02x] \n", *code_pos, instructions[I_NEG], I_NEG);
      code[(*code_pos)++] = I_NEG;
    }
  }
}

void write_instructions(AST_Node* tree, int32_t*code) {
  int code_pos = 0;
  write_instructions_rec(tree, code, &code_pos);
  fprintf(stdout, "%04x %10s[%02x] \n", code_pos, instructions[I_STOP], I_STOP);
  code[code_pos++] = I_STOP;
}

void dump_tokens(Token*token_list) {
  Token *cur = token_list;
  while(cur) {
    printf("TOKEN kind=%d value=%s\n", cur->kind, token_to_string(cur));
    cur = cur->next;
  }
}

#define CODE_SIZE 4096
#define DATA_SIZE 128
int code[4096];
int data[4096];

int main(int argc, char**args) {
  bool keep_going = true;
  Buffer buf;
  buf.size = INPUT_SIZE_MAX;
  buf.content = input;
  while(keep_going) { 
    Token * token_list = NULL;
    printf("\nvm> ");
    if (!fgets(buf.content, buf.size, stdin)) 
      break;
    token_list = scan_input(&buf);
    if (!token_list) {
      continue;;
    }
    //dump_tokens(token_list);
    log_trace("calling parse\n");
    AST_Node *root = begin_parsing(token_list);
    if (!root) {
      fputs("Syntax error", stderr);
    } else {
      print_postfix(root);
      puts("\n");
      write_instructions(root, code);
      free_tree(root);
      init(code, CODE_SIZE, data, DATA_SIZE);
      execute(true);
      state_dump();
    }
    free_token_list(token_list);
  }
}

