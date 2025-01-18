all: MemoryManager.all

MemoryManager.o: MemoryManager.h
	g++ MemoryManager.cpp -g -c -o MemoryManager.o

MemoryManager.a: MemoryManager.o
	ar rcs libMemoryManager.a MemoryManager.o

make clean:
	rm libMemoryManager.a MemoryManager.o

	