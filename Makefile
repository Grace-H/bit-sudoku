CC = gcc
CFLAGS = -g -Wall -std=gnu99

.PHONY: all clean

all: sudoku stack-solver

sudoku: sudoku.c util.o
	$(CC) $(CFLAGS) -o sudoku sudoku.c util.o

stack-solver: stack-solver.c util.o
	$(CC) $(CFLAGS) -o stack-solver stack-solver.c util.o

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -f *.o
	rm -f sudoku
