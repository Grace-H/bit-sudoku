CC = gcc
CFLAGS = -g -Wall -std=gnu99 -fstack-protector-all
OFLAGS = -std=gnu99 -pg

.PHONY: all clean

all: sudoku stack-solver ss-opt

sudoku: sudoku.c util.o
	$(CC) $(CFLAGS) -o sudoku sudoku.c util.o

stack-solver: stack-solver.c util.o
	$(CC) $(CFLAGS) -o stack-solver stack-solver.c util.o

ss-opt: ss-opt.c util.o
	$(CC) $(OFLAGS) -o ss-opt ss-opt.c util.o

util.o: util.c
	$(CC) $(OFLAGS) -c util.c

clean:
	rm -f *.o
	rm -f sudoku stack-solver ss-opt
