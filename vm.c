#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// instructions
#define I_NOP 0
#define I_STOP 1
#define I_PUSH 2
#define I_ADD 3
#define I_INC 4
#define I_DEC 5
#define I_JNZ 6
#define I_LOADPUSH 7
#define I_POPSTORE 8
#define I_STORE    9
#define I_CALL     10
#define I_RETURN    11
#define I_FRPUSH    12
#define I_FRPOP    13
#define I_JZ 14
#define I_POP 15
#define I_JMP 16

char *instructions[] = {
  "NOP",
  "STOP",
  "PUSH",
  "ADD",
  "INC",
  "DEC",
  "JNZ",
  "LOADPUSH",
  "POPSTORE",
  "STORE",
  "CALL",
  "RETURN",
  "FRPUSH", // push from Frame-pointer relative address in stack (i.e. read local variable)
  "FRPOP",  // pop to Frame-pointer relative address in stack (i.e. write local variable)
  "JZ",
  "POP",
  "JMP",  // unconditional relative jump
};

int args[256] = {
  0, // nop
  0, // stop
  1, // push
  0, // add
  0, // inc 
  0, // dec
  1, // jnz
  1, // loadpush
  1, // popstore
  1, // store
  2, // call
  0, // return
  1, // frpush
  1, // frpop
  1, // jnz
  0, // pop
  1, // jmp
};

#define DATA_SIZE 8192
#define CODE_SIZE 8192
#define STACK_SIZE 8192

int32_t data[DATA_SIZE] = { 7, 5 };
int32_t stack[STACK_SIZE];
int32_t code[CODE_SIZE] = {

  I_PUSH, 5,
  I_PUSH, 7,
  I_CALL, 12, 2,
  I_STOP,
  // def return_1()
  I_PUSH, 1,
  I_RETURN,
  I_NOP,
  // def multiply(x,y) {
  I_PUSH,    0,  // int total = 0;
  // while (x !=0 ) {
  I_FRPUSH, -6,
  I_JZ, +12, // exit loop
  I_DEC,         // x--
  I_FRPOP, -6,   // store x
  I_FRPUSH, 0,
  I_FRPUSH, -5,
  I_ADD,
  I_FRPOP, 0,
  I_JMP, -16,
  // }
  I_POP,
  I_RETURN
};

int32_t ip = 0;  // instruction pointer
int32_t sp = -1; // stack pointer
int32_t fp = 0;  // frame pointer

void execute(bool);
void trace_it(int32_t);
void state_dump();

int main(int argc, char**argv) {
  printf("START:\n");
  execute(true);
  state_dump();
  exit(0);
}

void execute(bool trace) {
  int32_t x;
  int32_t y;
  int32_t opcode;
  bool fatal = false;
  while (ip < CODE_SIZE && !fatal) {
    opcode = code[ip];
    if (trace) {
      trace_it(ip);
    }
    ip++;
    switch(opcode) {
      case I_STOP:
        return;
      case I_PUSH:
        stack[++sp] = code[ip++];
        break;
      case I_POP:
        sp--;
        break;
      case I_ADD:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          x = stack[sp--];
          y = stack[sp];
          stack[sp] = x + y;
        }
        break;
      case I_INC:
        stack[sp]++;
        break;
      case I_DEC:
        stack[sp]--;
        break;
      case I_LOADPUSH:
        stack[++sp] = data[code[ip++]];
        break;
      case I_POPSTORE:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          data[code[ip++]] = stack[sp--];
        }
        break;
      case I_FRPUSH:
        y = fp + code[ip++];
        stack[++sp] = stack[y];
        break;
      case I_FRPOP:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = fp + code[ip++];
          printf("frpop: y= %d\n", y);
          stack[y] = stack[sp--];
        }
        break;
      case I_STORE:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          data[code[ip++]] = stack[sp];
        }
        break;
      case I_JNZ:
        y = code[ip++];
        if (stack[sp]) {
          ip += y;
        }
        break;
      case I_JZ:
        y = code[ip++];
        if (!stack[sp]) {
          ip += y;
        }
        break;
      case I_JMP:
        y = code[ip++];
        ip += y;
        break;
      case I_CALL: {
        int32_t dest = code[ip++];
        int32_t arg_count = code[ip++];
        int32_t old_sp = sp;
        stack[++sp] = arg_count;
        stack[++sp] = old_sp;;
        stack[++sp] = ip;
        stack[++sp] = fp;
        ip = dest;
        printf("Call: ip updated to %d\n", ip);
        fp = sp + 1;
        break;
      }
      case I_RETURN: {
        int32_t return_value = stack[sp--];
        int32_t old_fp = fp;
        sp = stack[old_fp-3] - stack[old_fp-4];
        ip = stack[old_fp-2];
        fp = stack[old_fp-1];
        stack[++sp] = return_value;
        break;
      }
      default:
        printf("Failure: Invalid opcode %d", opcode);
        return;
    }
  }
}

void trace_it(int32_t ip) {
  int opcode = code[ip];
  state_dump();
  printf("\n ip=%d, sp=%d, fp=%d\n", ip, sp, fp);
  printf("%04x %10s ", ip, instructions[opcode]);
  int arg_count = args[opcode];
  if (arg_count >= 1) {
    printf("%4d", code[ip+1]);
  } else {
    printf("    ");
  }
  if (arg_count >= 2) {
    printf(", %4d", code[ip+2]);
  } else {
    printf("      ");
  }
}

void state_dump() {
  int t;
  printf("\nSTACK: [ ");
  for (t = 0; t <= sp; t++) {
    printf("%d ", stack[t]);
  }
  puts("]");
  printf("DATA:  [ ");
  for (t = 0; t <= 20; t++) {
    printf("%d ", data[t]);
  }
  puts("]");
}
  
