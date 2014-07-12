
all: interp vm

CC = c99
OBJECTS = vm.o 

vm : $(OBJECTS) opcodes.h
	$(CC) -o vm $(OBJECTS)

interp: interp.o
	$(CC) -o interp interp.o

.PHONY: clean

clean:
	-rm vm $(OBJECTS)
