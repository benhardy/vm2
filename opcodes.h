#ifndef OPCODES_H_INCLUDED
#define OPCODES_H_INCLUDED

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
#define I_MUL 17
#define I_NEG 18
#define I_DIV 19

#define I_MOD 20
#define I_SUB 21
/*
 * Human readable representations of the opcodes
 */
extern char *instructions[];
/*
 * Number of immediate arguments taken by each operation
 */
extern int args[256];

#endif
