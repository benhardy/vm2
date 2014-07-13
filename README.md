vm2
===

a tiny vm experiment in C

Why?
----

Because my C is super rusty and I could use the practice. With that in mind, please note that this code is probably rather awful-looking to an experienced C programmer. I'm just doing this for my own continuing education.

What it does now
----------------

* Interprets opcodes
* Opcodes can have arguments, or not
* The only data type is 32-bit signed integers
* Integer arithmetic
* Jumping, branching, function calls
* Stack operations
* Does most operations on the top of the stack. e.g. to add two operands, push them onto the stack and then ADD. 
   The operands will be popped and the result pushed onto the stack.
* Dispatches opcodes using a big old switch statement (currently)
* Has a trace mode so you can watch it step through execution
* A simple parser & compiler to turn arithmetic expressions into bytecode.
* A REPL (called interp) that compiles expressions to bytecode and then executes them on the VM
* A demo program written in the opcode language that calculates factorials recursively

Coming up next
-------------------------

* compiler: symbol tables (variable naming), function definition
* Memory protection (right now you can stack over/underflow, write data to bad addresses, etc)
* Error handling

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
* Lambdas, closures, environment context and all that jazz
