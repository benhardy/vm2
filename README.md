vm2
===

a tiny vm experiment in C

Why?
----

Because my C is super rusty and I could use the practice.

What it does now
----------------

* Interprets opcodes
* Implements a few opcodes
* Opcodes can have arguments, or not
* Does most operations on the top of the stack. e.g. to add two operands, push them onto the stack and then ADD. 
   The operands will be popped and the result pushed onto the stack.
* So far there's enough to be able to add, multiply, inc, dec, branch and call functions
* Recursion is supported
* Contains a little demo program that calculates factorials
* Has a trace mode so you can watch it step through execution
* The only data type is 32-bit signed integers
* Dispatches opcodes using a big old switch statement

Coming up next
-------------------------

* Memory protection (right now you can stack over/underflow, write data to bad addresses, etc)
* Error handling
* A simple compiler to turn expressions into bytecode

Later on
--------

* An assembler, because calculating and recalculating relative branch addresses by hand is not fun
* A simple JIT compiler to turn bytecode into actual machine code (or at least C)
* non-integer data types?
* support some native system calls for I/O

Much later
----------

* Memory allocation and garbage collection
* Start thinking about actual language source syntax
* Lambdas
