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

static int cells_str(uint16_t cells[GRP_SZ][GRP_SZ], char *buf, int n) {
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

static void singles(uint16_t cells[GRP_SZ][GRP_SZ]) {
  // Look for hidden singles in each row
  for (int i = 0; i < GRP_SZ; i++) {
    int opts_count[GRP_SZ];
    for (int x = 0; x < GRP_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < GRP_SZ; j++) {
      for (int k = 0; k < GRP_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    uint16_t singles = 0;
    for (int k = 0; k < GRP_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int j = 0; j < GRP_SZ; j++) {
      if (cells[i][j] & singles) {
        cells[i][j] &= singles;
        singles ^= cells[i][j]; // Cross off as safeguard
      }
    }
  }

  // Column
  for (int j = 0; j < GRP_SZ; j++) {
    // Count number of cells that can hold each number
    int opts_count[GRP_SZ];
    for (int x = 0; x < GRP_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int i = 0; i < GRP_SZ; i++) {
      for (int k = 0; k < GRP_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Create bitvector of numbers with only one possibility
    uint16_t singles = 0;
    for (int k = 0; k < GRP_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int i = 0; i < GRP_SZ; i++) {
      if (cells[i][j] & singles) {
        cells[i][j] &= singles;
        singles ^= cells[i][j]; // Cross off as safeguard
      }
    }
  }

  // Square
  for (int z = 0; z < GRP_SZ; z++) {
    int opts_count[GRP_SZ];
    for (int x = 0; x < GRP_SZ; x++) {
      opts_count[x] = 0;
    }

    int z1, z2;
    sqr_coords(z, &z1, &z2);
    for (int i = z1; i < z1 + CELL; i++) {
      for (int j = z2; j < z2 + CELL; j++) {
        for (int k = 0; k < GRP_SZ; k++) {
          opts_count[k] += (cells[i][j] >> k) & 1;
        }
      }
    }

    uint16_t singles = 0;
    for (int k = 0; k < GRP_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int i = z1; i < z1 + CELL; i++) {
      for (int j = z2; j < z2 + CELL; j++) {
        if (cells[i][j] & singles) {
          cells[i][j] &= singles;
          singles ^= cells[i][j]; // Cross off as safeguard
        }
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
        int a, b;
        sqr_coords(sqr_index(i, j), &a, &b);
        for (int k = a; k < a + CELL; k++) {
          for (int l = b; l < b + CELL; l++) {
            if ((i != k && j != l) && cells[i][j] == cells[k][l]) {
              for (int z1 = a; z1 < a + CELL; z1++) {
                for (int z2 = b; z2 < b + CELL; z2++) {
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

// Apply hidden pairs strategy: look for pairs of cells in each group
// that are the only ones that can have 2 options
static void hidden_pairs(uint16_t cells[GRP_SZ][GRP_SZ]) {
  // Look for hidden pairs in each row
  for (int i = 0; i < GRP_SZ; i++) {
    // Count number of cells that can hold each number
    int opts_count[GRP_SZ];
    for (int x = 0; x < GRP_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < GRP_SZ; j++) {
      for (int k = 0; k < GRP_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < GRP_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if(bit_count(pairs) >= 2) {
      char buf[16];
      vec_str(pairs, buf, 16);
      printf("pairs in row: %s\n", buf);
      // Find pairs of cells that share two of the possibilities
      for (int j = 0; j < GRP_SZ; j++) {
        uint16_t inter = cells[i][j] & pairs;
        if (bit_count(inter) == 2) {
          vec_str(inter, buf, 16);
          printf("cell %d,%d with pair found: %s\n", i,j,buf);
          for (int x = j + 1; x < GRP_SZ; x++) {
            if (inter == (cells[i][x] & pairs)) {
              LOG("hidden pair found %d,%d!\n", i,x);

              char buf[GRP_SZ * GRP_SZ + GRP_SZ];
              cells_str(cells, buf, GRP_SZ * GRP_SZ + GRP_SZ);
              LOG("%s\n", buf);
              cells[i][j] = inter;
              cells[i][x] = inter;
              for (int k = 0; k < GRP_SZ; k++) {
                if (k != j && k != x) {
                  cells[i][k] &= ~cells[i][j];
                }
              }
              pairs &= ~inter;
            }
          }
        }
      }
    }
  }
  /*
  // Column
  for (int j = 0; j < GRP_SZ; j++) {
// Count number of cells that can hold each number
int opts_count[GRP_SZ];
for (int x = 0; x < GRP_SZ; x++) {
opts_count[x] = 0;
}

for (int i = 0; i < GRP_SZ; i++) {
for (int k = 0; k < GRP_SZ; k++) {
opts_count[k] += (cells[i][j] >> k) & 1;
}
}

// Make bitvector of numbers that can only go in two cells
uint16_t pairs = 0;
for (int k = 0; k < GRP_SZ; k++) {
if (opts_count[k] == 2) {
pairs |= 1 << k;
}
}

// Find pairs of cells that share two of the possibilities
for (int i = 0; i < GRP_SZ; i++) {
uint16_t inter = cells[i][j] & pairs;
if (bit_count(inter) == 2) {
for (int y = i + 1; y < GRP_SZ; y++) {
if (inter == (cells[y][j] & pairs)) {
cells[i][j] = inter;
cells[y][j] = inter;
for (int k = 0; k < GRP_SZ; k++) {
if (k != i && k != y) {
cells[k][j] &= ~cells[i][j];
}
}
pairs &= ~inter;
}
}
}
}
}

// Square
for (int z = 0; z < GRP_SZ; z++) {
// Count number of cells that can hold each number
int opts_count[GRP_SZ];
for (int x = 0; x < GRP_SZ; x++) {
opts_count[x] = 0;
}

int z1, z2;
sqr_coords(z, &z1, &z2);
for (int i = z1; i < z1 + CELL; i++) {
for (int j = z2; j < z2 + CELL; j++) {
for (int k = 0; k < GRP_SZ; k++) {
opts_count[k] += (cells[i][j] >> k) & 1;
}
}
}

// Make bitvector of numbers that can only go in two cells
uint16_t pairs = 0;
for (int k = 0; k < GRP_SZ; k++) {
if (opts_count[k] == 2) {
pairs |= 1 << k;
}
}

// Find pairs of cells that share two of the possibilities
for (int i = z1; i < z1 + CELL; i++) {
for (int j = z2; j < z2 + CELL; j++) {
  uint16_t inter = cells[i][j] & pairs;
  if (bit_count(inter) == 2) {
    for (int x = z1; x < z1 + CELL; x++) {
      for (int y = z2; y < z2 + CELL; y++) {
        if ((i != x) && (j != y) && inter == (cells[x][y] & pairs)) {
          cells[i][j] = inter;
          cells[x][y] = inter;
          for (int a = z1; a < z1 + CELL; a++) {
            for (int b = z2; b < z2 + CELL; b++) {
              if ((a != i && b != j) && (a != x && b != y)) {
                cells[a][b] &= ~cells[i][j];
              }
            }
          }
          pairs &= ~inter;
        }
      }
    }
  }
}
}
}*/
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

  update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);

  for (int i = 0; i < 40; i++) {
    // Do one round of elimination
    eliminate(cells, rowfin, colfin, sqrfin);
    //    char buf[GRP_SZ * GRP_SZ + GRP_SZ];
    //    cells_str(cells, buf, GRP_SZ * GRP_SZ + GRP_SZ);
    //    LOG("cells after eliminate round %d\n%s", i, buf);
    // naked_pairs(cells);
    hidden_pairs(cells);
    singles(cells);
    //    cells_str(cells, buf, GRP_SZ * GRP_SZ + GRP_SZ);
    //    LOG("cells after singles\n%s", buf);
    update_solved((const uint16_t(*)[GRP_SZ]) cells, rowfin, colfin, sqrfin);

    if (is_solved(rowfin, colfin, sqrfin)) {
      printf("Solved in %d iterations\n", i);
      return 0;
    }
  }

  printf("Not solved\n");

  return 1;
}

/* vim:set ts=2 sw=2 et: */
