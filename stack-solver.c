/**
 * stack-solver.c
 *
 * Stack-based sudoku solver. Applies transformations to the sudoku grid until it
 * becomes invalid, then reverts the previous change to attempt a different solution.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

#define N_CELLS 81

struct transform {
	int i; // Coordinates of cell transformed
	int j;
	uint16_t candidates; // Former candidates of cell
};

// Get square number (0->9 reading left-right top-bottom) from i,j coordinates
// Square index is i rounded down to nearest multiple of cell size + j divided by cell size
static inline int sqr_index(int i, int j) {
  return (i / CELL) * CELL + j / CELL;
}

// Get i,j coordinates of top left cell in a square from its index
static void sqr_coords(int n, int *i, int *j) {
  *i = (n / CELL) * CELL;
  *j = (n % CELL) * CELL;
}

// Update which values in each row, column, and square have been solved
// Set bit indicates value exists in region
static void update_solved(const uint16_t cells[GRP_SZ][GRP_SZ], uint16_t row[GRP_SZ],
    uint16_t col[GRP_SZ], uint16_t sqr[GRP_SZ]) {
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      uint16_t c = cells[i][j];
      // if only one number is in bitvector (power of 2), mark it as found
      if (!(c & (c - 1))) {
        row[i] |= c;
        col[j] |= c;
        sqr[sqr_index(i, j)] |= c;
      }
    }
  }
}

// Check if board is solved - each row/column/square is solved
// if the xor of all cells is 0x1ff (1 bit set)
int is_solved(uint16_t row[GRP_SZ], uint16_t col[GRP_SZ], uint16_t sqr[GRP_SZ]) {
  uint16_t target = (1 << GRP_SZ) - 1;
  for (int i = 0; i < GRP_SZ; i++) {
    if (row[i] != target || col[i] != target || sqr[i] != target) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }

  // Initialize bitvectors of cell possibilites
  uint16_t cells[GRP_SZ][GRP_SZ];
  uint16_t nine = (1 << GRP_SZ) - 1;
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      cells[i][j] = nine; 
    }
  }

  // Read & parse puzzle from file
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("open");
    return 1;
  }

  char buf[GRP_SZ];
  for (int i = 0; i < GRP_SZ; i++) {
    int ret = fscanf(f, "%9c\n", buf);
    if (ret < 1) {
      perror("fgets");
      return 1;
    }

    for (int j = 0; j < GRP_SZ; j++) {
      if (buf[j] != '0') {
        char d = buf[j];
        if (d >= '1' && d <= '9') {
          cells[i][j] = 1 << (d - '1');
        } else {
          fprintf(stderr, "Invalid digit: %d\n", d);
          return 1;
        }
      }
    }
  }

  // Cross off candidates in all unsolved cells
  uint16_t rowfin[GRP_SZ];
  uint16_t colfin[GRP_SZ];
  uint16_t sqrfin[GRP_SZ];

  memset(&rowfin, 0, GRP_SZ * sizeof(uint16_t));
  memset(&colfin, 0, GRP_SZ * sizeof(uint16_t));
  memset(&sqrfin, 0, GRP_SZ * sizeof(uint16_t));

  update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);

  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      // if this cell is not solved, cross off possibilities
      if(cells[i][j] & (cells[i][j] - 1)) {
        cells[i][j] &= ~rowfin[i];
        cells[i][j] &= ~colfin[j];
        cells[i][j] &= ~sqrfin[sqr_index(i, j)];
      }
    }
  }

  // Initialize stack for tracking transformations
	struct stack transforms;
  stack_init(&transforms);

  int n = 0; // Current location in grid
  while (n < N_CELLS && !is_solved(rowfin, colfin, sqrfin)) {

  }

	return 0;
}

/* vim:set ts=2 sw=2 et: */
