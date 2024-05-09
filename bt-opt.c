/**
 * bt.c - simple backtracking algorithm implementation, as described here:
 * https://en.wikipedia.org/wiki/Sudoku_solving_algorithms
 *
 * @author: Grace-H
 */
#include <stdio.h>
#include "util.h"

// Represents a cell with a priority based on initial candidate count
struct cell {
  int i;
  int j;
  int priority;
};

/** Priority function for use with priority queue */
int priority(struct cell *cell) {
  return cell->priority;
}

/**
 * Get block number (0->9 reading left-right top-bottom) from i,j coordinates
 */
static inline int blk_index(int i, int j) {
  return (i / BLK_WIDTH) * BLK_WIDTH + j / BLK_WIDTH;
}

/**
 * Get i,j coordinates of top left cell in a block from its index
 */
static void blk_coords(int n, int *i, int *j) {
  *i = (n / BLK_WIDTH) * BLK_WIDTH;
  *j = (n % BLK_WIDTH) * BLK_WIDTH;
}

// Eliminate as candidate value of solved cell & propagate any other
// solved cells process creates
void remove_candidate(uint16_t cells[HOUSE_SZ][HOUSE_SZ], int i, int j) {
  uint16_t elim = ~cells[i][j];

  for (int x = 0; x < HOUSE_SZ; x++) {
    if (j != x) {
      uint16_t old = cells[i][x];
      cells[i][x] &= elim;
      if ((old & (old - 1)) && !(cells[i][x] & (cells[i][x] - 1))) {
        remove_candidate(cells, i, x);
      }
    }
  }

  for (int y = 0; y < HOUSE_SZ; y++) {
    if (i != y) {
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
      if (!(a == i && b == j)) {
        uint16_t old = cells[a][b];
        cells[a][b] &= elim;
        if ((old & (old - 1)) && !(cells[a][b] & (cells[a][b] - 1))) {
          remove_candidate(cells, a, b);
        }
      }
    }
  }
}

/**
 * Apply a basic backtracking algorithm to solve.
 *
 * cells: array of uint16_t representing puzzle to solve
 * original: array of bitvectors indicating which indices in cells
 * 	contain values from the original puzzle
 * @return nonzero on failure to solve
 */
int solve(uint16_t cells[HOUSE_SZ][HOUSE_SZ], uint16_t original[HOUSE_SZ]) {
	uint16_t max = 1 << (HOUSE_SZ + 1);
	uint16_t target = max - 2;  // 1's in bits 1-9

	// Bitvectors of all values present in each row/column for conflict checking
	uint16_t row[HOUSE_SZ];
	uint16_t col[HOUSE_SZ];
	uint16_t blk[HOUSE_SZ];
  uint16_t candidates[HOUSE_SZ][HOUSE_SZ];
	for (int i = 0; i < HOUSE_SZ; i++) {
		row[i] = 0;
		col[i] = 0;
		blk[i] = 0;
    for (int j = 0; j < HOUSE_SZ; j++) {
      candidates[i][j] = target;
    }
	}

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (cells[i][j] != 1) {
        candidates[i][j] = cells[i][j];
        remove_candidate(candidates, i, j);
      }
    }
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (!(candidates[i][j] & (candidates[i][j] - 1))) {
        cells[i][j] = candidates[i][j];
        row[i] |= cells[i][j];
        col[j] |= cells[i][j];
        blk[blk_index(i, j)] |= cells[i][j];
      }
    }
  }

  struct pq pq;
  pq_init(&pq, (int (*)(void *)) priority, N_CELLS);

  struct cell priorities[HOUSE_SZ][HOUSE_SZ];
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      priorities[i][j].i = i;
      priorities[i][j].j = j;
      priorities[i][j].priority = 9 - bit_count(candidates[i][j]);
      if (priorities[i][j].priority < 8)
        pq_insert(&pq, &priorities[i][j]);
    }
  }

  struct stack done;
  stack_init(&done);

	int delta = 1;  // Direction to change n
	while ((delta && !pq_is_empty(&pq)) || (!delta && !stack_is_empty(&done))) {
    struct cell *cell;
    if (delta) {
      cell = pq_extract_max(&pq);
    } else {
      cell = stack_pop(&done);
    }
		int i = cell->i;
		int j = cell->j;
		int z = blk_index(i, j);

    // Cell is partially solved, undo previously tried solution
    if (cells[i][j] > 1) {
      row[i] ^= cells[i][j];
      col[j] ^= cells[i][j];
      blk[z] ^= cells[i][j];
    }

    // Calculate new solution
    cells[i][j] <<= 1;
    while (cells[i][j] < max) {
      // No conflict with existing solved cells
      if (!(cells[i][j] & row[i] || cells[i][j] & col[j] || cells[i][j] & blk[z])) {
        row[i] |= cells[i][j];
        col[j] |= cells[i][j];
        blk[z] |= cells[i][j];
        stack_push(&done, cell);
        delta = 1;
        break;
      }
      cells[i][j] <<= 1;
    }
    if (cells[i][j] >= max) {
      cells[i][j] = 1;
      pq_insert(&pq, cell);
      delta = 0;
    }
  }

	// Check if solved
	uint16_t solved = target;
	for (int i = 0; i < HOUSE_SZ; i++) {
		solved &= row[i];
		solved &= col[i];
		solved &= blk[i];
	}

	if (solved != target) {
		return 1;
	}
	return 0;
}

int main(int argc, char **argv) {

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <puzzle file>\n", argv[0]);
		return 1;
	}

	// Initialize all cells to 1
	uint16_t cells[HOUSE_SZ][HOUSE_SZ];
	uint16_t original[HOUSE_SZ];
	for (int i = 0; i < HOUSE_SZ; i++) {
		original[i] = 0;
		for (int j = 0; j < HOUSE_SZ; j++) {
			cells[i][j] = 1;
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
			uint16_t d = buf[j] - '0';
			if (d > 0 && d <= 9) {
				cells[i][j] = 1 << d;
				original[i] |= (1 << j);
			} else if (d != 0) {
				fprintf(stderr, "Invalid digit: %d\n", d);
				fclose(f);
				return 1;
			}
		}
	}

	fclose(f);

	return solve(cells, original);
}

/* vim:set ts=2 sw=2 et: */
