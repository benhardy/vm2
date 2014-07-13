
all: interp vm

CC = c99

opcodes.o: opcodes.c opcodes.h
	$(CC) -o opcodes.o -c opcodes.c

interp.o: interp.c opcodes.h
	$(CC) -o interp.o -c interp.c

vm.o: vm.c opcodes.h
	$(CC) -o vm.o -c vm.c

vm: vm.o opcodes.o
	$(CC) -o vm vm.o opcodes.o

interp: interp.o opcodes.o
	$(CC) -o interp interp.o opcodes.o

.PHONY: clean

clean:
	-rm vm *.o interp
