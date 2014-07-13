
all: interp demo

CC = c99

demo.o: demo.c vm.h opcodes.h
	$(CC) -o demo.o     -c demo.c

demo: demo.o vm.o opcodes.o
	$(CC) -o demo   demo.o   vm.o opcodes.o

interp: interp.o opcodes.o vm.o
	$(CC) -o interp interp.o vm.o opcodes.o

opcodes.o: opcodes.c opcodes.h
	$(CC) -o opcodes.o -c opcodes.c

interp.o: interp.c opcodes.h
	$(CC) -o interp.o   -c interp.c

vm.o: vm.c opcodes.h
	$(CC) -o vm.o       -c vm.c

.PHONY: clean

clean:
	-rm vm *.o interp
