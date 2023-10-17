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
  uint16_t solution;   // Current, tried solution of cell
  uint16_t candidates; // Former candidates of cell
  uint16_t tried;      // Candidates that have been tried as solutions
  uint16_t **cells;    // Copy of cells before this trans applied
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

// Eliminate as candidate value of solved cell & propagate any other
// solved cells process creates
void init_rm_candidate(uint16_t cells[HOUSE_SZ][HOUSE_SZ], int i, int j) {
  uint16_t elim = ~cells[i][j];

  for (int x = 0; x < HOUSE_SZ; x++) {
    if (cells[i][x] & (cells[i][x] - 1)) {
      cells[i][x] &= elim;
      if (!(cells[i][x] & (cells[i][x] - 1))) {
        init_rm_candidate(cells, i, x);
      }
    }
  }

  for (int y = 0; y < HOUSE_SZ; y++) {
    if (cells[y][j] & (cells[y][j] - 1)) {
      cells[y][j] &= elim;
      if (!(cells[y][j] & (cells[y][j] - 1))) {
        init_rm_candidate(cells, y, j);
      }
    }
  }

  int z1, z2;
  blk_coords(blk_index(i, j), &z1, &z2);
  for (int a = z1; a < z1 + BLK_WIDTH; a++) {
    for (int b = z2; b < z2 + BLK_WIDTH; b++) {
      if (cells[a][b] & (cells[a][b] - 1)) {
        cells[a][b] &= elim;
        if (!(cells[a][b] & (cells[a][b] - 1))) {
          init_rm_candidate(cells, a, b);
        }
      }
    }
  }
}

// Eliminate as candidate value of solved cell & propagate any other
// solved cells process creates
// Difference from init_rm: Won't touch before n, fails fast if error
int attempt_rm_candidate(uint16_t cells[HOUSE_SZ][HOUSE_SZ], int i, int j, int n) {
  int ni = n / HOUSE_SZ;
  int nj = n % HOUSE_SZ;
  int nz1, nz2;
  blk_coords(blk_index(ni, nj), &nz1, &nz2);

  uint16_t elim = ~cells[i][j];

  for (int x = i == ni ? nj + 1 : 0; x < HOUSE_SZ; x++) {
    if (x != j) {
      uint16_t old_candidates = cells[i][x];
      cells[i][x] &= elim;
      if (!cells[i][x]) {
        return 1;
      }
      if ((old_candidates & (old_candidates - 1)) && !(cells[i][x] & (cells[i][x] - 1))) {
        if (attempt_rm_candidate(cells, i, x, n)) {
          return 1;
        }
      }
    }
  }

  for (int y = j == nj ? ni + 1 : 0; y < HOUSE_SZ; y++) {
    if (y != i) {
      uint16_t old_candidates = cells[y][j];
      cells[y][j] &= elim;
      if (!cells[y][j]) {
        return 1;
      }
      if ((old_candidates & (old_candidates - 1)) && !(cells[y][j] & (cells[y][j] - 1))) {
        if (attempt_rm_candidate(cells, y, j, n)) {
          return 1;
        }
      }
    }
  }

  int z1, z2;
  blk_coords(blk_index(i, j), &z1, &z2);
  int same_block = nz1 == z1 && nz2 == z2;
  for (int a = same_block ? ni : z1; a < z1 + BLK_WIDTH; a++) {
    for (int b = z2; b < z2 + BLK_WIDTH; b++) {
      if ((same_block && (a > ni || (a == ni && b > nj)) && !(a == i && b == j)) || 
          (!same_block && !(a == i && b == j))) {
        uint16_t old_candidates = cells[a][b];
        cells[a][b] &= elim;
        if (!cells[a][b]) {
          return 1;
        }
        if ((old_candidates & (old_candidates - 1)) && !(cells[a][b] & (cells[a][b] - 1))) {
          if (attempt_rm_candidate(cells, a, b, n)) {
            return 1;
          }
        }
      }
    }
  }
  if (!is_valid(cells)) {
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <puzzle file>\n", argv[0]);
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
      perror("fscanf");
      fclose(f);
      return 1;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
      if (buf[j] != '0') {
        char d = buf[j];
        if (d >= '1' && d <= '9') {
          cells[i][j] = 1 << (d - '1');
          init_rm_candidate(cells, i, j);
        } else {
          fprintf(stderr, "Invalid digit: %d\n", d);
          fclose(f);
          return 1;
        }
      }
    }
  }

  fclose(f);

  // Cross off candidates in all unsolved cells
  uint16_t rowfin[HOUSE_SZ];
  uint16_t colfin[HOUSE_SZ];
  uint16_t blkfin[HOUSE_SZ];

  memset(&rowfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&colfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&blkfin, 0, HOUSE_SZ * sizeof(uint16_t));

  update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);

  // Initialize stack for tracking transformations
  struct stack transforms;
  stack_init(&transforms);

  int n = 0; // Current location in grid
  while (n < N_CELLS && !is_solved(rowfin, colfin, blkfin)) {
    struct transform *trans = NULL;

    // Perform transformation
    if (is_valid((const uint16_t(*)[HOUSE_SZ]) cells)) {
      // If valid, choose next unsolved cell
      int i = n / HOUSE_SZ;
      int j = n % HOUSE_SZ;
      while ((n < N_CELLS) && !(cells[i][j] & (cells[i][j] - 1))) {
        n++;
        i = n / HOUSE_SZ;
        j = n % HOUSE_SZ;
      }

      if (n == N_CELLS)
        break;

      // Construct transformation
      int solution = 0;
      while (!((cells[i][j] >> solution) & 1)) {
        solution++;
      }

      trans = malloc(sizeof(struct transform));
      trans->i = i;
      trans->j = j;
      trans->solution = 1 << solution;
      trans->candidates = cells[i][j];
      trans->tried = 1 << solution;
      trans->cells = malloc(HOUSE_SZ * HOUSE_SZ * sizeof(uint16_t));
      copy_cells(cells, (uint16_t(*)[HOUSE_SZ]) trans->cells);
      cells[i][j] = trans->solution;
      stack_push(&transforms, trans);
    } else {
      // Revert to first prior transformation on cell with untried candidates
      do {
        if (trans) {
          free(trans->cells);
          free(trans);
        }

        trans = stack_pop(&transforms);

        if (!trans) {
          printf("Not solved\n");
          return 1;
        }
      } while ((trans->candidates & ~trans->tried) == 0);

      n = trans->i * HOUSE_SZ + trans->j;

      copy_cells((uint16_t(*)[HOUSE_SZ]) trans->cells, cells);

      // Construct transformation
      int solution = 0;
      uint16_t remaining = trans->candidates & ~trans->tried;
      while (!((remaining >> solution) & 1)) {
        solution++;
      }

      trans->solution = 1 << solution;
      trans->tried |= trans->solution;
      cells[trans->i][trans->j] = trans->solution;
      stack_push(&transforms, trans);
    }
    attempt_rm_candidate(cells, trans->i, trans->j, n);

    update_solved((const uint16_t(*)[HOUSE_SZ]) cells, rowfin, colfin, blkfin);
  }

  printf("Solved");

  struct transform *trans = NULL;
  while ((trans = stack_pop(&transforms))) {
    free(trans->cells);
    free(trans);
  }
  return 0;
}

/* vim:set ts=2 sw=2 et: */
