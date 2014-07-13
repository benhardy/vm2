#include "opcodes.h"
#include "vm.h"

#define DATA_SIZE 8192
#define CODE_SIZE 8192

int32_t data[DATA_SIZE] = { 7, 5 };
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

int main(int argc, char**argv) {
  printf("START:\n");
  init(code, CODE_SIZE, data, DATA_SIZE);
  execute(true);
  state_dump();
  exit(0);
}

