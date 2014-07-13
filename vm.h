#ifndef VM_H_INCLUDED
#define VM_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

extern void init(int32_t*code, int code_size, int32_t*data, int data_size);
extern void execute(bool);
extern void trace_it(int32_t);
extern void state_dump();

#endif

