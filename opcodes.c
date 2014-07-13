
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
  "MUL",  // multiply top two things on stack, pop them, push result
  "NEG",  // negate top of stack
  "DIV",  // divide stack[sp-1] / stack[sp],

  "MOD",  // modulo stack[sp-1] % stack[sp]
  "SUB"   // subtract stack[sp-1] - stack[sp]
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
  0, // neg
  0, // div
  0, // mod
  0  // sub
};

