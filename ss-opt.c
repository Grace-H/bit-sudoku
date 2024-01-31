/**
 * ss-opt.c
 *
 * Optimized stack-based sudoku solver. Applies transformations to the sudoku grid until it
 * becomes invalid, then reverts the previous change to attempt a different solution.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

#define N_CELLS 81

struct cell {
  int i;
  int j;
  int priority;  // 9 - Initial number of candidates
};

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


// Eliminate as candidate value of solved cell & propagate any other
// solved cells process creates
void remove_candidate(uint16_t cells[HOUSE_SZ][HOUSE_SZ], int i, int j) {
  uint16_t elim = ~cells[i][j];

  for (int x = 0; x < HOUSE_SZ; x++) {
    if (j != x && cells[i][x]) {
      uint16_t old = cells[i][x];
      cells[i][x] &= elim;
      if ((old & (old - 1)) && !(cells[i][x] & (cells[i][x] - 1))) {
        remove_candidate(cells, i, x);
      }
    }
  }

  for (int y = 0; y < HOUSE_SZ; y++) {
    if (i != y && cells[y][j]) {
      uint16_t old = cells[y][j];
      cells[y][j] &= elim;
      if ((old & (old - 1)) && !(cells[y][j] & (cells[y][j] - 1))) {
        remove_candidate(cells, y, j);
      }
    }
  }

  int z1, z2;
  blk_coords(blk_index(i, j), &z1, &z2);
  for (int a = z1; a < z1 + BLK_WIDTH; a++) {
    for (int b = z2; b < z2 + BLK_WIDTH; b++) {
      if (!(a == i && b == j) && cells[a][b]) {
        uint16_t old = cells[a][b];
        cells[a][b] &= elim;
        if ((old & (old - 1)) && !(cells[a][b] & (cells[a][b] - 1))) {
          remove_candidate(cells, a, b);
        }
      }
    }
  }
}

// Check if the board is valid - all cells have at least one candidate
int is_valid(const uint16_t cells[HOUSE_SZ][HOUSE_SZ]) {
	uint16_t max = 1 << HOUSE_SZ;
	uint16_t row[HOUSE_SZ];
	uint16_t col[HOUSE_SZ];
	uint16_t blk[HOUSE_SZ];
	for (int i = 0; i < HOUSE_SZ; i++) {
		row[i] = 0;
		col[i] = 0;
		blk[i] = 0;
	}

	for (int i = 0; i < HOUSE_SZ; i++) {
		for (int j = 0; j < HOUSE_SZ; j++) {
			row[i] |= cells[i][j];
			col[j] |= cells[i][j];
			blk[blk_index(i, j)] |= cells[i][j];
		}
	}

	uint16_t target = max - 1;  // 1's in bits 1-9
	uint16_t solved = target;
	for (int i = 0; i < HOUSE_SZ; i++) {
		solved &= row[i];
		solved &= col[i];
		solved &= blk[i];
	}

	if (solved == target) {
		return 1;
	}
	return 0;
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
    int backtracks = 0;
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

    // Construct priority queue--worklist for cells
    // Cells with fewer candidates are higher priority
    struct cell priorities[HOUSE_SZ][HOUSE_SZ];
    int priority(struct cell *cell) {
      return cell->priority;
    }

    struct pq worklist;
    pq_init(&worklist, (int (*)(void *)) priority, N_CELLS);
    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int j = 0; j < HOUSE_SZ; j++) {
        priorities[i][j].i = i;
        priorities[i][j].j = j;
        priorities[i][j].priority = bit_count(cells[i][j]);
        if (priorities[i][j].priority > 1)
          pq_insert(&worklist, &priorities[i][j]);
      }
    }

    // Initialize stack for tracking transformations
    struct stack transforms;
    stack_init(&transforms);

    // Get next cell in worklist
    while (!pq_is_empty(&worklist)) {
      struct transform *trans = NULL;

      // Perform transformation
      struct cell *cell = pq_extract_max(&worklist);
      int i = cell->i;
      int j = cell->j;

      // If it has remaining candidates
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
        pq_insert(&worklist, &priorities[i][j]);

        // Revert to first prior transformation on cell with untried candidates
        do {
          if (trans) {
            pq_insert(&worklist, &priorities[trans->i][trans->j]);
            free(trans->cells);
            free(trans);
          }

          trans = stack_pop(&transforms);
          backtracks++;

          if (!trans) {
            return 1;
          }
        } while ((trans->candidates & ~trans->tried) == 0);

        //assert(((struct cell *) pq_extract_max(&worklist))->i == trans->i);
        // n = trans->i * HOUSE_SZ + trans->j;

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

    fprintf(stdout, "%d", backtracks);

    struct transform *trans = NULL;
    while ((trans = stack_pop(&transforms))) {
      free(trans->cells);
      free(trans);
    }

    // Terminate early on failure
    if (!is_valid(cells))
      return 1;
  }
  return 0;
}

/* vim:set ts=2 sw=2 et: */
