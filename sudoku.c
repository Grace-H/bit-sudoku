/* sudoku.c
 * Sudoku solver, representing cell possibilities as a bitvector
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define GRP_SZ 9
#define CELL 3
#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n",       \
				 __func__, __LINE__, ##__VA_ARGS__)

static int cells_str(const uint16_t cells[GRP_SZ][GRP_SZ], char *buf, int n) {
  if (n < (GRP_SZ + 1) * GRP_SZ) {
    LOG("too short");
    errno = EINVAL;
    return -1;
  }

  int k = 0;
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ + 1; j++) {
      if (j == GRP_SZ) {
        buf[k++] = '\n';
      }
      else if (!(cells[i][j] & (cells[i][j] - 1))) {
        int x = 1;
        while (cells[i][j] >> x != 0)
          x++;
        buf[k++] = x + '0';
      }
      else {
        buf[k++] = '0';
      }
    }
  }
  buf[k] = '\0';
  return k;
}

static int vec_str(const uint16_t vec, char *buf, int n) {
  if (n < GRP_SZ + 1) {
    errno = EINVAL;
    return -1;
  }

  int i;
  for (i = 0; i < GRP_SZ; i++) {
    buf[i] = ((vec >> (GRP_SZ - i - 1)) & 1) + '0';
  }

  buf[i] = '\0';
  return i;
}

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

// Count number of set bits in vector of length GRP_SZ
static int bit_count(const uint16_t n) {
  int count = 0;
  for (int i = 0; i < GRP_SZ; i++) {
    count += (n >> i) & 1;
  }
  return count;
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

// Eliminate possibilites for each cell based on what values are already
// in each row/column/square
static void eliminate(uint16_t cells[GRP_SZ][GRP_SZ], const uint16_t row[GRP_SZ],
		      const uint16_t col[GRP_SZ], const uint16_t sqr[GRP_SZ]) {
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      // if this cell is not solved, cross off possibilities
      if(cells[i][j] & (cells[i][j] - 1)) {
        cells[i][j] &= ~row[i];
        cells[i][j] &= ~col[j];
        cells[i][j] &= ~sqr[sqr_index(i, j)];
      }
    }
  }
}

// Use naked pairs strategy to eliminate further options
// Naked pair: two cells in same group that have only two identical possibilities
static void naked_pairs(uint16_t cells[GRP_SZ][GRP_SZ]) {

  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      if (bit_count(cells[i][j]) == 2) {
        // Check for pairs in remainder of row, eliminating possibiliities
        // from row if so
        for (int k = j + 1; k < GRP_SZ; k++) {
          // same pair
          if (cells[i][j] == cells[i][k]) {
            for (int x = 0; x < GRP_SZ; x++) {
              if (x != j && x != k) {
                cells[i][x] &= ~cells[i][j];
              }
            }
            break; // there won't (shouldn't) be another pair
          }
        }

        // Column
        for (int k = i + 1; k < GRP_SZ; k++) {
          if (cells[i][j] == cells[k][j]) {
            for (int y = 0; y < GRP_SZ; y++) {
              if (y != i && y != k) {
                cells[y][j] &= ~cells[i][j];
              }
            }
            break;
          }
        }

        // Square
        for (int k = (i / CELL) * CELL; k < (i / CELL + 1) * CELL; k++) {
          for (int l = (j / CELL) * CELL; l < (j / CELL + 1) * CELL; l++) {
            if ((i != k && j != l) && cells[i][j] == cells[k][l]) {
              for (int z1 = (i / CELL) * CELL; z1 < ((i + 1) / CELL) * CELL; z1++) {
                for (int z2 = (j / CELL) * CELL; z2 < ((i + 1) / CELL) * CELL; z2++) {
                  if (cells[i][j] != cells[z1][z2]) {
                    cells[z1][z2] &= ~cells[i][j];
                  }
                }
              }
              break;
            }
          }
        }
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
  uint16_t cells[GRP_SZ][GRP_SZ]; // only the first 9 bits will be used
  for (int i = 0; i < GRP_SZ; i++) {
    for (int j = 0; j < GRP_SZ; j++) {
      cells[i][j] = (1 << GRP_SZ) - 1;
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

  // Cross off initial round of bits
  uint16_t rowfin[GRP_SZ];
  uint16_t colfin[GRP_SZ];
  uint16_t sqrfin[GRP_SZ];

  memset(&rowfin, 0, GRP_SZ * sizeof(uint16_t));
  memset(&colfin, 0, GRP_SZ * sizeof(uint16_t));
  memset(&sqrfin, 0, GRP_SZ * sizeof(uint16_t));

  update_solved(cells, rowfin, colfin, sqrfin);

  for (int i = 0; i < 20; i++) {
    // Do one round of elimination
    eliminate(cells, rowfin, colfin, sqrfin);
    //    char buf[GRP_SZ * GRP_SZ + GRP_SZ];
    //    cells_str(cells, buf, GRP_SZ * GRP_SZ + GRP_SZ);
    //    LOG("cells after eliminate round %d\n%s", i, buf);
    naked_pairs(cells);
    cells_str(cells, buf, ROW * ROW + ROW);
    LOG("cells after naked pairs \n%s", buf);
    update_solved(cells, rowfin, colfin, sqrfin);

    if (is_solved(rowfin, colfin, sqrfin)) {
      printf("Solved in %d iterations\n", i);
      return 0;
    }
  }

  printf("Not solved\n");

  return 1;
}
