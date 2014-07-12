
all: interp vm

OBJECTS = vm.o 

vm : $(OBJECTS) opcodes.h
	cc -o vm $(OBJECTS)

interp: interp.o
	cc -o interp interp.o

.PHONY: clean

clean:
	-rm vm $(OBJECTS)
