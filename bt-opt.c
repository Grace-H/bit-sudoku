/**
 * bt.c - simple backtracking algorithm implementation, as described here:
 * https://en.wikipedia.org/wiki/Sudoku_solving_algorithms
 *
 * @author: Grace-H
 */
#include <stdio.h>
#include "util.h"

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

	// Bitvectors of all values present in each row/column for conflict checking
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

	int n = 0;
	int delta = 1;  // Direction to change n
	while (n < N_CELLS && n >= 0) {
		int i = n / HOUSE_SZ;
		int j = n % HOUSE_SZ;
		int z = blk_index(i, j);

		// If not part of the original puzzle
		if (!((original[i] >> j) & 1)) {
			if (cells[i][j] > 1) {
				row[i] ^= cells[i][j];
				col[j] ^= cells[i][j];
				blk[z] ^= cells[i][j];
			}

			while (cells[i][j] < max) {
				cells[i][j] <<= 1;
				// No conflict with existing solved cells
				if (!(cells[i][j] & row[i] || cells[i][j] & col[j] || cells[i][j] & blk[z])) {
					row[i] |= cells[i][j];
					col[j] |= cells[i][j];
					blk[z] |= cells[i][j];
					delta = 1;
					break;
				}
			}
			if (cells[i][j] >= max) {
				cells[i][j] = 1;
				delta = -1;
			}
		}
		n += delta;
	}

	// Check if solved
	uint16_t target = max - 2;  // 1's in bits 1-9
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

	// Initialize all cells to 0
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
