CC = gcc
CFLAGS = -g -Wall -Werror -std=gnu99

.PHONY: all clean

all: sudoku

sudoku: sudoku.c
	$(CC) $(CFLAGS) -o sudoku sudoku.c

clean:
	rm -f *.o
	rm -f sudoku
