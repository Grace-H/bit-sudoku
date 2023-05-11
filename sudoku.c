/* sudoku.c
 * Sudoku solver, representing cell possibilities as a bitvector
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ROW 9
#define CELL 3
#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n",	\
				 __func__, __LINE__, ##__VA_ARGS__)

// Get square number (0->9 reading left-right top-bottom) from i,j coordinates
// Square index is i rounded down to nearest multiple of cell size + j divided by cell size
static inline int sqr_index(int i, int j) {
  return (i / CELL) * CELL + j / CELL;
}

// Count number of set bits in vector of length ROW
static int bit_count(uint16_t n) {
  int count = 0;
  for (int i = 0; i < ROW; i++) {
    count += (n >> i) & 1;
  }
  return count;
}

// Update which values in each row, column, and square have been solved
// Set bit indicates value exists in region
static void update_solved(const uint16_t cells[ROW][ROW], uint16_t row[ROW], uint16_t col[ROW], uint16_t sqr[ROW]) {
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
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
int is_solved(uint16_t row[ROW], uint16_t col[ROW], uint16_t sqr[ROW]) {
  uint16_t target = (1 << ROW) - 1;
  for (int i = 0; i < ROW; i++) {
    if (row[i] != target || col[i] != target || sqr[i] != target) {
      return 0;
    }
  }
  return 1;
}

// Eliminate possibilites for each cell based on what values are already
// in each row/column/square
static void eliminate(uint16_t cells[ROW][ROW], const uint16_t row[ROW], const uint16_t col[ROW], const uint16_t sqr[ROW]) {
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
      // if this cell is not solved, cross off possibilities
      if(cells[i][j] & (cells[i][j] - 1)) {
	cells[i][j] &= ~row[i];
	cells[i][j] &= ~col[j];
	cells[i][j] &= ~sqr[sqr_index(i, j)];
      }
    }
  }
}

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }

  // Initialize bitvectors of cell possibilites
  uint16_t cells[ROW][ROW]; // only the first 9 bits will be used
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
      cells[i][j] = (1 << ROW) - 1;
    }
  }

  // Read & parse puzzle from file
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("open");
    return 1;
  }

  char buf[ROW];
  for (int i = 0; i < ROW; i++) {
    int ret = fscanf(f, "%9c\n", buf);
    if (ret < 1) {
      perror("fgets");
      return 1;
    }

    for (int j = 0; j < ROW; j++) {
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

  // Cross off initial round of bits
  uint16_t rowfin[ROW];
  uint16_t colfin[ROW];
  uint16_t sqrfin[ROW];

  memset(&rowfin, 0, ROW * sizeof(uint16_t));
  memset(&colfin, 0, ROW * sizeof(uint16_t));
  memset(&sqrfin, 0, ROW * sizeof(uint16_t));

  update_solved(cells, rowfin, colfin, sqrfin);

  for (int i = 0; i < 20; i++) {
    // Do one round of elimination
    eliminate(cells, rowfin, colfin, sqrfin);
    update_solved(cells, rowfin, colfin, sqrfin);

    if (is_solved(rowfin, colfin, sqrfin)) {
      printf("Solved in %d iterations\n", i);
      return 0;
    }
  }

  printf("Not solved\n");

  return 0;
}
