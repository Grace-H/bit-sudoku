CC = gcc
CFLAGS = -g -Wall -std=gnu99

.PHONY: all clean

all: sudoku

sudoku: sudoku.c util.o
	$(CC) $(CFLAGS) -o sudoku sudoku.c util.o

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -f *.o
	rm -f sudoku
