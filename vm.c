#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "vm.h"
#include "opcodes.h"

#define STACK_SIZE 8192


int32_t ip = 0;  // instruction pointer
int32_t sp = -1; // stack pointer
int32_t fp = 0;  // frame pointer

int32_t _stack[STACK_SIZE];
int32_t* _code;
int32_t* _data;
int _code_size;
int _data_size;

void init(int32_t*code, int code_size, int32_t*data, int data_size) {
  _code = code;
  _data = data;
  _code_size = code_size;
  _data_size = data_size;
  ip = 0;
  sp = -1;
  fp = 0;
}

void execute(bool trace) {
  int32_t x;
  int32_t y;
  int32_t opcode;
  bool fatal = false;
  while (ip < _code_size && !fatal) {
    opcode = _code[ip];
    if (trace) {
      trace_it(ip);
    }
    ip++;
    switch(opcode) {
      case I_STOP:
        return;
      case I_PUSH:
        _stack[++sp] = _code[ip++];
        break;
      case I_POP:
        sp--;
        break;
      case I_ADD:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = _stack[sp--];
          x = _stack[sp];
          _stack[sp] = x + y;
        }
        break;
      case I_MUL:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = _stack[sp--];
          x = _stack[sp];
          _stack[sp] = x * y;
        }
        break;
      case I_DIV:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = _stack[sp--];
          x= _stack[sp];
          _stack[sp] = x / y;
        }
        break;
      case I_MOD:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = _stack[sp--];
          x = _stack[sp];
          _stack[sp] = x % y;
        }
        break;
      case I_SUB:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = _stack[sp--];
          x = _stack[sp];
          _stack[sp] = x / y;
        }
        break;
      case I_INC:
        _stack[sp]++;
        break;
      case I_NEG:
        _stack[sp] = - _stack[sp];;
        break;
      case I_DEC:
        _stack[sp]--;
        break;
      case I_LOADPUSH:
        _stack[++sp] = _data[_code[ip++]];
        break;
      case I_POPSTORE:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          _data[_code[ip++]] = _stack[sp--];
        }
        break;
      case I_FRPUSH:
        y = fp + _code[ip++];
        _stack[++sp] = _stack[y];
        break;
      case I_FRPOP:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          y = fp + _code[ip++];
          _stack[y] = _stack[sp--];
        }
        break;
      case I_STORE:
        if (sp < 0) {
          printf("Stack underflow");
          fatal = true;
        } else {
          _data[_code[ip++]] = _stack[sp];
        }
        break;
      case I_JNZ:
        y = _code[ip++];
        if (_stack[sp]) {
          ip += y;
        }
        break;
      case I_JZ:
        y = _code[ip++];
        if (!_stack[sp]) {
          ip += y;
        }
        break;
      case I_JMP:
        y = _code[ip++];
        ip += y;
        break;
      case I_CALL: {
        int32_t dest = _code[ip++];
        int32_t arg_count = _code[ip++];
        int32_t old_sp = sp;
        _stack[++sp] = arg_count;
        _stack[++sp] = old_sp;;
        _stack[++sp] = ip;
        _stack[++sp] = fp;
        ip = dest;
        fp = sp + 1;
        break;
      }
      case I_RETURN: {
        int32_t return_value = _stack[sp--];
        int32_t old_fp = fp;
        sp = _stack[old_fp-3] - _stack[old_fp-4];
        ip = _stack[old_fp-2];
        fp = _stack[old_fp-1];
        _stack[++sp] = return_value;
        break;
      }
      default:
        printf("Failure: Invalid opcode %d", opcode);
        return;
    }
  }
}

void trace_it(int32_t ip) {
  int opcode = _code[ip];
  state_dump();
  printf("    REGS: ip=%d, sp=%d, fp=%d\n", ip, sp, fp);
  printf("%04x %10s ", ip, instructions[opcode]);
  int arg_count = args[opcode];
  if (arg_count >= 1) {
    printf("%4d", _code[ip+1]);
  } else {
    printf("    ");
  }
  if (arg_count >= 2) {
    printf(", %4d", _code[ip+2]);
  } else {
    printf("      ");
  }
  puts("\n");
}

void state_dump() {
  int t;
  printf("   STACK: [ ");
  for (t = 0; t <= sp; t++) {
    printf("%d ", _stack[t]);
  }
  puts("]");
  return;
  printf("    DATA: [ ");
  for (t = 0; t <= 20; t++) {
    printf("%d ", _data[t]);
  }
  puts("]");
}
  
