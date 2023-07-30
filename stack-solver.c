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
    row[i] = 0;
    col[i] = 0;
    sqr[i] = 0;
  }

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

// Check if the board is valid - all cells have at least one possibility
int is_valid(const uint16_t cells[GRP_SZ][GRP_SZ]) {
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
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

  // Initialize copy for reference of original puzzle
  uint16_t ref[GRP_SZ][GRP_SZ];
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      ref[i][j] = cells[i][j];
    }
  }

  // Initialize stack for tracking transformations
  struct stack transforms;
  stack_init(&transforms);

  int n = 0; // Current location in grid
  while (n < N_CELLS && !is_solved(rowfin, colfin, sqrfin)) {
    while (is_valid((const uint16_t(*)[GRP_SZ]) cells)) {
      // Find next unsolved cell
      int i = n / GRP_SZ;
      int j = n % GRP_SZ;
      while ((n < N_CELLS) && !(cells[i][j] & (cells[i][j] - 1))) {
        n++;
        i = n / GRP_SZ;
        j = n % GRP_SZ;
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
      cells[i][j] &= 1 << solution;
      stack_push(&transforms, trans);

      // Update houses
      uint16_t elim = ~(1 << solution);

      for (int x = 0; x < GRP_SZ; x++) {
        if (x != j) {
          cells[i][x] &= elim;
        }
      }

      for (int y = 0; y < GRP_SZ; y++) {
        if (y != i) {
          cells[y][j] &= elim;
        }
      }

      int z1, z2;
      sqr_coords(sqr_index(i, j), &z1, &z2);
      for (int a = z1; a < z1 + CELL; a++) {
        for (int b = z2; b < z2 + CELL; b++) {
          if (!(a == i && b == j)) {
            cells[a][b] &= elim;
          }
        }
      }

      // This change may have resulted in other cells being solved
      update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);

      for (int a = 0; a < GRP_SZ; a++) {
        for (int b = 0; b < GRP_SZ; b++) {
          // if this cell is not solved, cross off possibilities
          if(cells[a][b] & (cells[a][b] - 1)) {
            cells[a][b] &= ~rowfin[a];
            cells[a][b] &= ~colfin[b];
            cells[a][b] &= ~sqrfin[sqr_index(a, b)];
          }
        }
      }
    }

    // Revert changes back to valid state
    // Change previous transformation, or roll back as many as needed
    int reverted = 0;
    while (!reverted) {
      struct transform *trans = stack_pop(&transforms);
      int i = trans->i;
      int j = trans->j;
      n = i * GRP_SZ + j;
      uint16_t solution = cells[i][j]; // Current, invalid solution

      // Reverse effect on houses
      for (int x = 0; x < GRP_SZ; x++) {
        if (x != j) {
          cells[i][x] |= solution;
        }
      }

      for (int y = 0; y < GRP_SZ; y++) {
        if (y != i) {
          cells[y][j] |= solution;
        }
      }

      int z1, z2;
      sqr_coords(sqr_index(i, j), &z1, &z2);
      for (int a = z1; a < z1 + CELL; a++) {
        for (int b = z2; b < z2 + CELL; b++) {
          if (!(a == i && b == j)) {
            cells[a][b] |= solution;
          }
        }
      }

      update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);

      uint16_t nine = (1 << GRP_SZ) - 1;
      for (int a = 0; a < GRP_SZ; a++) {
        for (int b = 0; b < GRP_SZ; b++) {
          // if this cell is not solved, cross off possibilities
          if(cells[a][b] != ref[a][b]) {
            cells[a][b] = (nine & (~rowfin[a] | ~colfin[b] | ~sqrfin[sqr_index(a, b)]));
          }
        }
      }

      // Try alternate transformation on the same cell
      cells[i][j] = trans->candidates;
      int shift = 0;
      while (solution >> shift) {
        shift++;
      }
      shift++;
      while (shift < GRP_SZ && !((cells[i][j] >> shift) & 1)) {
        // XXX off by one? ^
        shift++;
      }

      if (shift >= GRP_SZ) {
        free(trans);
        // TODO - need to try alternate transformation with the last one
        // because otherwise puzzle appears valid and advance step repeats
        // identically
      } else {
        // Try alternate transformation
        cells[i][j] &= 1 << shift;
        stack_push(&transforms, trans);

        // Update houses
        uint16_t elim = ~(1 << shift);

        for (int x = 0; x < GRP_SZ; x++) {
          if (x != j) {
            cells[i][x] &= elim;
          }
        }

        for (int y = 0; y < GRP_SZ; y++) {
          if (y != i) {
            cells[y][j] &= elim;
          }
        }

        int z1, z2;
        sqr_coords(sqr_index(i, j), &z1, &z2);
        for (int a = z1; a < z1 + CELL; a++) {
          for (int b = z2; b < z2 + CELL; b++) {
            if (!(a == i && b == j)) {
              cells[a][b] &= elim;
            }
          }
        }
        reverted = 1;
      }
    }

    update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);
  }

  printf("Solved");
  return 0;
}

/* vim:set ts=2 sw=2 et: */
