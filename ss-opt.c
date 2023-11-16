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
  uint16_t (* cells)[HOUSE_SZ];    // Copy of cells before this trans applied
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

// Check if the board is valid - all cells have at least one candidate
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
void remove_candidate(uint16_t cells[HOUSE_SZ][HOUSE_SZ], int i, int j) {
  uint16_t elim = ~cells[i][j];

  for (int x = 0; x < HOUSE_SZ; x++) {
    cells[i][x] &= elim;
  }

  for (int y = 0; y < HOUSE_SZ; y++) {
    cells[y][j] &= elim;
  }

  int z1, z2;
  blk_coords(blk_index(i, j), &z1, &z2);
  for (int a = z1; a < z1 + BLK_WIDTH; a++) {
    for (int b = z2; b < z2 + BLK_WIDTH; b++) {
      cells[a][b] &= elim;
    }
  }
}

// Hidden singles strategy
static void singles(uint16_t cells[HOUSE_SZ][HOUSE_SZ]) {
  // Count options in each house
  int row_counts[HOUSE_SZ][HOUSE_SZ];
  int col_counts[HOUSE_SZ][HOUSE_SZ];
  int blk_counts[HOUSE_SZ][HOUSE_SZ];
  uint16_t row_singles[HOUSE_SZ];
  uint16_t col_singles[HOUSE_SZ];
  uint16_t blk_singles[HOUSE_SZ];
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      row_counts[i][j] = 0;
      col_counts[i][j] = 0;
      blk_counts[i][j] = 0;
    }
    row_singles[i] = 0;
    col_singles[i] = 0;
    blk_singles[i] = 0;
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        int val = (cells[i][j] >> k) & 1;
        row_counts[i][k] += val;
        col_counts[j][k] += val;
        blk_counts[blk_index(i, j)][k] += val;
      }
    }
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int k = 0; k < HOUSE_SZ; k++) {
      uint16_t k_shift = 1 << k;
      if (row_counts[i][k] == 1) {
        row_singles[i] |= k_shift;
      }
      if (col_counts[i][k] == 1) {
        col_singles[i] |= k_shift;
      }
      if (blk_counts[i][k] == 1) {
        blk_singles[i] |= k_shift;
      }
    }
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (cells[i][j] & (cells[i][j] - 1)) {
        if (cells[i][j] & row_singles[i]) {
          cells[i][j] &= row_singles[i];
          remove_candidate(cells, i, j);
          break;
        }
        if (cells[i][j] & col_singles[j]) {
          cells[i][j] &= col_singles[j];
          remove_candidate(cells, i, j);
          break;
        }
        int blk = blk_index(i, j);
        if (cells[i][j] & blk_singles[blk]) {
          cells[i][j] &= blk_singles[blk];
          remove_candidate(cells, i, j);
          break;
        }
      }
    }
  }
}


int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <puzzle file> ...\n", argv[0]);
    return 1;
  }

  for (int file = 1; file < argc; file++) {
    // Initialize bitvectors of cell possibilites
    uint16_t cells[HOUSE_SZ][HOUSE_SZ];
    uint16_t nine = (1 << HOUSE_SZ) - 1;
    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int j = 0; j < HOUSE_SZ; j++) {
        cells[i][j] = nine;
      }
    }

    // Read & parse puzzle from file
    FILE *f = fopen(argv[file], "r");
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
            remove_candidate(cells, i, j);
          } else {
            fprintf(stderr, "Invalid digit: %d\n", d);
            fclose(f);
            return 1;
          }
        }
      }
    }

    fclose(f);

    singles(cells);

    // Initialize stack for tracking transformations
    struct stack transforms;
    stack_init(&transforms);

    int n = 0; // Current location in grid
    while (n < N_CELLS) {
      struct transform *trans = NULL;

      // Perform transformation
      int i = n / HOUSE_SZ;
      int j = n % HOUSE_SZ;
      while ((n < N_CELLS) && !(cells[i][j] & (cells[i][j] - 1))) {
        n++;
        i = n / HOUSE_SZ;
        j = n % HOUSE_SZ;
      }

      if (n == N_CELLS) {
        break;
      }

      if (cells[i][j]) {

        // Construct transformation
        uint16_t solution = 1;
        while (!(cells[i][j] & solution)) {
          solution <<= 1;
        }

        trans = malloc(sizeof(struct transform));
        trans->i = i;
        trans->j = j;
        trans->solution = solution;
        trans->candidates = cells[i][j];
        trans->tried = solution;
        trans->cells = malloc(HOUSE_SZ * HOUSE_SZ * sizeof(uint16_t));
        copy_cells(cells, trans->cells);
      } else {
        // Revert to first prior transformation on cell with untried candidates
        do {
          if (trans) {
            free(trans->cells);
            free(trans);
          }

          trans = stack_pop(&transforms);

          if (!trans) {
            return 1;
          }
        } while ((trans->candidates & ~trans->tried) == 0);

        n = trans->i * HOUSE_SZ + trans->j;

        copy_cells(trans->cells, cells);

        // Construct transformation
        uint16_t solution = 1;
        uint16_t remaining = trans->candidates ^ trans->tried;
        while (!(remaining & solution)) {
          solution <<= 1;
        }

        trans->solution = solution;
        trans->tried |= solution;
      }

      stack_push(&transforms, trans);
      cells[trans->i][trans->j] = trans->solution;
      remove_candidate(cells, trans->i, trans->j);
    }


    struct transform *trans = NULL;
    while ((trans = stack_pop(&transforms))) {
      free(trans->cells);
      free(trans);
    }
  }
  return 0;
}

/* vim:set ts=2 sw=2 et: */
