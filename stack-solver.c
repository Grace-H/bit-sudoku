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
  uint16_t tried;      // Candidates that have been tried as solutions
};

// Get block number (0->9 reading left-right top-bottom) from i,j coordinates
// Block index is i rounded down to nearest multiple of cell size + j divided by cell size
static inline int blk_index(int i, int j) {
  return (i / BLK_WIDTH) * BLK_WIDTH + j / BLK_WIDTH;
}

// Get i,j coordinates of top left cell in a block from its index
static void blk_coords(int n, int *i, int *j) {
  *i = (n / BLK_WIDTH) * BLK_WIDTH;
  *j = (n % BLK_WIDTH) * BLK_WIDTH;
}

// Determine cell number (nth cell) from i,j
static inline int cell_index(int i, int j) {
  return i * HOUSE_SZ + j;
}

// Update which values in each row, column, and block have been solved
// Set bit indicates value exists in region
static void update_solved(const uint16_t cells[HOUSE_SZ][HOUSE_SZ], uint16_t row[HOUSE_SZ],
                          uint16_t col[HOUSE_SZ], uint16_t blk[HOUSE_SZ]) {

  for (int i = 0; i < HOUSE_SZ; i++) {
    row[i] = 0;
    col[i] = 0;
    blk[i] = 0;
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      uint16_t c = cells[i][j];
      // if only one number is in bitvector (power of 2), mark it as found
      if (!(c & (c - 1))) {
        row[i] |= c;
        col[j] |= c;
        blk[blk_index(i, j)] |= c;
      }
    }
  }
}

// Check if board is solved - each row/column/block is solved
// if the xor of all cells is 0x1ff (1 bit set)
int is_solved(uint16_t row[HOUSE_SZ], uint16_t col[HOUSE_SZ], uint16_t blk[HOUSE_SZ]) {
  uint16_t target = (1 << HOUSE_SZ) - 1;
  for (int i = 0; i < HOUSE_SZ; i++) {
    if (row[i] != target || col[i] != target || blk[i] != target) {
      return 0;
    }
  }
  return 1;
}

// Check if the board is valid - all cells have at least one possibility
int is_valid(const uint16_t cells[HOUSE_SZ][HOUSE_SZ]) {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (!cells[i][j]) {
        return 0;
      }
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
  uint16_t cells[HOUSE_SZ][HOUSE_SZ];
  uint16_t nine = (1 << HOUSE_SZ) - 1;
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      cells[i][j] = nine;
    }
  }

  // Read & parse puzzle from file
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("open");
    return 1;
  }

  char buf[HOUSE_SZ];
  for (int i = 0; i < HOUSE_SZ; i++) {
    int ret = fscanf(f, "%9c\n", buf);
    if (ret < 1) {
      perror("fgets");
      return 1;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
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
  uint16_t rowfin[HOUSE_SZ];
  uint16_t colfin[HOUSE_SZ];
  uint16_t blkfin[HOUSE_SZ];

  memset(&rowfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&colfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&blkfin, 0, HOUSE_SZ * sizeof(uint16_t));

  update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      // if this cell is not solved, cross off possibilities
      if(cells[i][j] & (cells[i][j] - 1)) {
        cells[i][j] &= ~rowfin[i];
        cells[i][j] &= ~colfin[j];
        cells[i][j] &= ~blkfin[blk_index(i, j)];
      }
    }
  }

  // Initialize copy for reference of original puzzle
  uint16_t ref[HOUSE_SZ][HOUSE_SZ];
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      ref[i][j] = cells[i][j];
    }
  }

  // Initialize stack for tracking transformations
  struct stack transforms;
  stack_init(&transforms);

  int n = 0; // Current location in grid
  while (n < N_CELLS && !is_solved(rowfin, colfin, blkfin)) {
    while (is_valid((const uint16_t(*)[HOUSE_SZ]) cells)) {
      // Find next unsolved cell
      int i = n / HOUSE_SZ;
      int j = n % HOUSE_SZ;
      while ((n < N_CELLS) && !(cells[i][j] & (cells[i][j] - 1))) {
        n++;
        i = n / HOUSE_SZ;
        j = n % HOUSE_SZ;
      }

      if (n >= N_CELLS) {
        printf("Not solved\n");
        return 1;
      }

      // Construct transformation
      int solution = 0;
      while (!((cells[i][j] >> solution) & 1)) {
        solution++;
      }

      struct transform *trans = malloc(sizeof(struct transform));
      trans->i = i;
      trans->j = j;
      trans->candidates = cells[i][j];
      trans->tried = 1 << solution;
      cells[i][j] &= 1 << solution;
      stack_push(&transforms, trans);

      // Update houses
      uint16_t elim = ~(1 << solution);

      for (int x = 0; x < HOUSE_SZ; x++) {
        if (x != j) {
          cells[i][x] &= elim;
        }
      }

      for (int y = 0; y < HOUSE_SZ; y++) {
        if (y != i) {
          cells[y][j] &= elim;
        }
      }

      int z1, z2;
      blk_coords(blk_index(i, j), &z1, &z2);
      for (int a = z1; a < z1 + BLK_WIDTH; a++) {
        for (int b = z2; b < z2 + BLK_WIDTH; b++) {
          if (!(a == i && b == j)) {
            cells[a][b] &= elim;
          }
        }
      }

      // This change may have resulted in other cells being solved
      update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);

      for (int a = 0; a < HOUSE_SZ; a++) {
        for (int b = 0; b < HOUSE_SZ; b++) {
          // if this cell is not solved, cross off possibilities
          if(cells[a][b] & (cells[a][b] - 1)) {
            cells[a][b] &= ~rowfin[a];
            cells[a][b] &= ~colfin[b];
            cells[a][b] &= ~blkfin[blk_index(a, b)];
          }
        }
      }
    }

    // Revert changes back to valid state
    // Identify first prior transformation on a cell with untried candidates
    struct transform *trans = NULL;
    do {
      if (trans) {
        free(trans);
      }

      trans = stack_pop(&transforms);

      if (!trans) {
        LOG("No remaining transformations--unable to revert further");
        exit(1);
      }

      int i = trans->i;
      int j = trans->j;
      n = i * HOUSE_SZ + j;
      uint16_t solution = cells[i][j]; // Current, invalid solution

      // Reverse effect on houses
      // Add as candidate in cells > n that weren't solved in original
      for (int x = j + 1; x < HOUSE_SZ; x++) {
        if (cells[i][x] != ref[i][x]) {
          cells[i][x] |= solution;
        }
      }

      for (int y = i + 1; y < HOUSE_SZ; y++) {
        if (cells[y][j] != ref[y][j]) {
          cells[y][j] |= solution;
        }
      }

      int z1, z2;
      blk_coords(blk_index(i, j), &z1, &z2);
      for (int a = i; a < z1 + BLK_WIDTH; a++) {
        for (int b = z2; b < z2 + BLK_WIDTH; b++) {
          if ((a == i && b > j) || a > i) {
            if (cells[a][b] != ref[a][b]) {
              cells[a][b] |= solution;
            }
          }
        }
      }

      update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);

      uint16_t nine = (1 << HOUSE_SZ) - 1;
      for (int a = 0; a < HOUSE_SZ; a++) {
        for (int b = 0; b < HOUSE_SZ; b++) {
          // if this cell is not solved, cross off possibilities
          if(cell_index(a, b) > n && cells[a][b] != ref[a][b]) {
            cells[a][b] = (nine & (~rowfin[a] | ~colfin[b] | ~blkfin[blk_index(a, b)]));
          }
        }
      }
    } while ((trans->candidates & (~trans->tried)) == 0);

    // TODO Try alternate transformation on the same cell

    update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);
  }

  printf("Solved");
  return 0;
}

/* vim:set ts=2 sw=2 et: */
