#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "opcodes.h"

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
  "MUL"
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
  0, // mul
};

#define DATA_SIZE 8192
#define CODE_SIZE 8192
#define STACK_SIZE 8192

int32_t data[DATA_SIZE] = { 7, 5 };
int32_t stack[STACK_SIZE];
int32_t code[CODE_SIZE] = {

  I_PUSH, 5,
  I_CALL, 33, 1,
  I_STOP,
  I_NOP, I_NOP,

  // def return_1()
// @8:
  I_PUSH, 1,
  I_RETURN,
  I_NOP,

// @12:
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
  I_RETURN,
  
  I_NOP,

// @33
  // factorial(n) { 
  I_FRPUSH, -5,
  I_JNZ, 4,     // if (!n)
  I_POP,
  I_PUSH, 1,
  I_RETURN,     //    return 1;
  I_DEC,        // n--;
  I_JNZ, 4,     // if (!n)
  I_POP,
  I_PUSH, 1,    //    return 1;
  I_RETURN,     
  I_CALL, 33, 1, // get fact(n-1)
  I_FRPUSH, -5,
  I_MUL,
  I_RETURN,
  
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
      case I_MUL:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          x = stack[sp--];
          y = stack[sp];
          stack[sp] = x * y;
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
  printf("    REGS: ip=%d, sp=%d, fp=%d\n", ip, sp, fp);
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
  puts("\n");
}

void state_dump() {
  int t;
  printf("   STACK: [ ");
  for (t = 0; t <= sp; t++) {
    printf("%d ", stack[t]);
  }
  puts("]");
  return;
  printf("    DATA: [ ");
  for (t = 0; t <= 20; t++) {
    printf("%d ", data[t]);
  }
  puts("]");
}
  
