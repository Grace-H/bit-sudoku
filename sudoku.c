/* sudoku.c
 * Sudoku solver, representing cell candidates as bitvectors
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static uint16_t cells[HOUSE_SZ][HOUSE_SZ]; // Candidates
static uint16_t solved[HOUSE_SZ][HOUSE_SZ]; // Solutions

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

// Update which values in each row, column, and block have been solved
// Set bit indicates value exists in house
static void update_solved(uint16_t row[HOUSE_SZ], uint16_t col[HOUSE_SZ],
    uint16_t blk[HOUSE_SZ]) {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      uint16_t c = solved[i][j];
      if (c) {
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

// Eliminate as candidate value of solved cell & propagate any other
// solved cells process creates
void remove_candidate(int i, int j) {
  uint16_t elim = ~cells[i][j];
  solved[i][j] = cells[i][j];
  cells[i][j] = 0;

  for (int x = 0; x < HOUSE_SZ; x++) {
    if (cells[i][x]) {
      cells[i][x] &= elim;
      if (!(cells[i][x] & (cells[i][x] - 1))) {
        remove_candidate(i, x);
      }
    }
  }

  for (int y = 0; y < HOUSE_SZ; y++) {
    if (cells[y][j]) {
      cells[y][j] &= elim;
      if (!(cells[y][j] & (cells[y][j] - 1))) {
        remove_candidate(y, j);
      }
    }
  }

  int z1, z2;
  blk_coords(blk_index(i, j), &z1, &z2);
  for (int a = z1; a < z1 + BLK_WIDTH; a++) {
    for (int b = z2; b < z2 + BLK_WIDTH; b++) {
      if (cells[a][b]) {
        cells[a][b] &= elim;
        if (!(cells[a][b] & (cells[a][b] - 1))) {
          remove_candidate(a, b);
        }
      }
    }
  }
}

// Eliminate possibilites for each cell based on what values are already
// in each row/column/block
static void eliminate(const uint16_t row[HOUSE_SZ], const uint16_t col[HOUSE_SZ],
    const uint16_t blk[HOUSE_SZ]) {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      // if this cell is not solved, cross off possibilities
      if(cells[i][j]) {
        cells[i][j] &= ~row[i];
        cells[i][j] &= ~col[j];
        cells[i][j] &= ~blk[blk_index(i, j)];
        if (!(cells[i][j] & (cells[i][j] - 1))) {
          remove_candidate(i, j);
        }
      }
    }
  }
}

// Hidden singles strategy
static void singles() {
  // Look for hidden singles in each row
  for (int i = 0; i < HOUSE_SZ; i++) {
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    uint16_t singles = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (cells[i][j] & singles) {
        cells[i][j] &= singles;
        remove_candidate(i, j);

        // Recalculate opts_count & singles
        // as they may have changed in propagation
        for (int x = 0; x < HOUSE_SZ; x++) {
          opts_count[x] = 0;
        }

        for (int l = 0; l < HOUSE_SZ; l++) {
          for (int k = 0; k < HOUSE_SZ; k++) {
            opts_count[k] += (cells[i][l] >> k) & 1;
          }
        }

        uint16_t singles = 0;
        for (int k = 0; k < HOUSE_SZ; k++) {
          if (opts_count[k] == 1) {
            singles |= 1 << k;
          }
        }
        j = 0;  // Go back to beginning
      }
    }
  }

  // Column
  for (int j = 0; j < HOUSE_SZ; j++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Create bitvector of numbers with only one possibility
    uint16_t singles = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int i = 0; i < HOUSE_SZ; i++) {
      if (cells[i][j] & singles) {
        cells[i][j] &= singles;
        remove_candidate(i, j);

        for (int x = 0; x < HOUSE_SZ; x++) {
          opts_count[x] = 0;
        }

        for (int l = 0; l < HOUSE_SZ; l++) {
          for (int k = 0; k < HOUSE_SZ; k++) {
            opts_count[k] += (cells[l][j] >> k) & 1;
          }
        }

        // Create bitvector of numbers with only one possibility
        uint16_t singles = 0;
        for (int k = 0; k < HOUSE_SZ; k++) {
          if (opts_count[k] == 1) {
            singles |= 1 << k;
          }
        }
        i = 0;
      }
    }
  }

  // Block
  for (int z = 0; z < HOUSE_SZ; z++) {
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    int z1, z2;
    blk_coords(z, &z1, &z2);
    for (int a = z1; a < z1 + BLK_WIDTH; a++) {
      for (int b = z2; b < z2 + BLK_WIDTH; b++) {
        for (int k = 0; k < HOUSE_SZ; k++) {
          opts_count[k] += (cells[a][b] >> k) & 1;
        }
      }
    }

    uint16_t singles = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 1) {
        singles |= 1 << k;
      }
    }

    // Find cells that have one of the hidden singles
    for (int a = z1; a < z1 + BLK_WIDTH; a++) {
      for (int b = z2; b < z2 + BLK_WIDTH; b++) {
        if (cells[a][b] & singles) {
          cells[a][b] &= singles;
          remove_candidate(a, b);

          for (int x = 0; x < HOUSE_SZ; x++) {
            opts_count[x] = 0;
          }

          for (int a2 = z1; a2 < z1 + BLK_WIDTH; a2++) {
            for (int b2 = z2; b2 < z2 + BLK_WIDTH; b2++) {
              for (int k = 0; k < HOUSE_SZ; k++) {
                opts_count[k] += (cells[a2][b2] >> k) & 1;
              }
            }
          }

          uint16_t singles = 0;
          for (int k = 0; k < HOUSE_SZ; k++) {
            if (opts_count[k] == 1) {
              singles |= 1 << k;
            }
          }
          a = z1;
          b = z2;
        }
      }
    }
  }
}

// Use naked pairs strategy to eliminate further options
// Naked pair: two cells in same house that have only two identical possibilities
static void naked_pairs() {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      if (bit_count(cells[i][j]) == 2) {
        // Check for pairs in remainder of row, eliminating possibiliities
        // from row if so
        for (int k = j + 1; k < HOUSE_SZ; k++) {
          // same pair
          if (cells[i][j] == cells[i][k]) {
            for (int x = 0; x < HOUSE_SZ; x++) {
              if (cells[i][x] && x != j && x != k) {
                cells[i][x] &= ~cells[i][j];
                if(!(cells[i][x] & (cells[i][x] - 1))) {
                  remove_candidate(i, x);
                }
              }
            }
            break; // there won't (shouldn't) be another pair
          }
        }

        // Column
        for (int k = i + 1; k < HOUSE_SZ; k++) {
          if (cells[i][j] == cells[k][j]) {
            for (int y = 0; y < HOUSE_SZ; y++) {
              if (cells[y][j] && y != i && y != k) {
                cells[y][j] &= ~cells[i][j];
                if (!(cells[y][j] & (cells[y][j] - 1))) {
                  remove_candidate(y, j);
                }
              }
            }
            break;
          }
        }

        // Block
        int z1, z2;
        blk_coords(blk_index(i, j), &z1, &z2);
        for (int a = i; a < z1 + BLK_WIDTH; a++) {
          for (int b = z2; b < z2 + BLK_WIDTH; b++) {
            if (a == i && b <= j)
              continue;

            if (cells[i][j] == cells[a][b]) {
              for (int k = z1; k < z1 + BLK_WIDTH; k++) {
                for (int l = z2; l < z2 + BLK_WIDTH; l++) {
                  if (cells[k][l] && cells[i][j] != cells[k][l]) {
                    cells[k][l] &= ~cells[i][j];
                    if (!(cells[k][l] & (cells[k][l] - 1))) {
                      remove_candidate(k, l);
                    }
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

// Apply hidden pairs strategy: look for pairs of cells in each house
// that are the only ones that can have 2 options
static void hidden_pairs() {

  // Look for hidden pairs in each row
  for (int i = 0; i < HOUSE_SZ; i++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if(bit_count(pairs) >= 2) {
      // Find pairs of cells that share two of the possibilities
      for (int j = 0; j < HOUSE_SZ; j++) {
        uint16_t inter = cells[i][j] & pairs;
        if (bit_count(inter) >= 2) {
          for (int x = j + 1; x < HOUSE_SZ; x++) {
            if (bit_count(inter & cells[i][x]) == 2) {
              cells[i][j] = inter;
              cells[i][x] = inter;
              pairs &= ~inter;
            }
          }
        }
      }
    }
  }

  // Column
  for (int j = 0; j < HOUSE_SZ; j++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if(bit_count(pairs) >= 2) {
      // Find pairs of cells that share two of the possibilities
      for (int i = 0; i < HOUSE_SZ; i++) {
        uint16_t inter = cells[i][j] & pairs;
        if (bit_count(inter) >= 2) {
          for (int y = i + 1; y < HOUSE_SZ; y++) {
            if (bit_count(inter & cells[y][j]) == 2) {
              cells[i][j] = inter;
              cells[y][j] = inter;
              pairs &= ~inter;
            }
          }
        }
      }
    }
  }

  // Block
  for (int z = 0; z < HOUSE_SZ; z++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    int z1, z2;
    blk_coords(z, &z1, &z2);
    for (int a = z1; a < z1 + BLK_WIDTH; a++) {
      for (int b = z2; b < z2 + BLK_WIDTH; b++) {
        for (int k = 0; k < HOUSE_SZ; k++) {
          opts_count[k] += (cells[a][b] >> k) & 1;
        }
      }
    }

    // Make bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if (bit_count(pairs) >= 2) {
      // Find pairs of cells that share two of the possibilities
      for (int a = z1; a < z1 + BLK_WIDTH; a++) {
        for (int b = z2; b < z2 + BLK_WIDTH; b++) {
          uint16_t inter = cells[a][b] & pairs;
          if (bit_count(inter) >= 2) {
            for (int x = z1; x < z1 + BLK_WIDTH; x++) {
              for (int y = z2; y < z2 + BLK_WIDTH; y++) {
                if (!(a == x && b == y) &&
                    bit_count(inter & cells[x][y]) >= 2) {
                  cells[a][b] = inter;
                  cells[x][y] = inter;
                  pairs &= ~inter;
                }
              }
            }
          }
        }
      }
    }
  }
}

// Claiming pairs strategy: Find pairs of cells in the same row/column
// that are in the same block, and eliminate that candidate from the block
static void claiming_pairs() {
  // Look for claiming pairs in each row
  for (int i = 0; i < HOUSE_SZ; i++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make a bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if (bit_count(pairs) >= 1) {
      // Find the pairs of cells with these numbers
      for (int j = 0; j < HOUSE_SZ; j++) {
        uint16_t inter = cells[i][j] & pairs;
        if (inter) {
          // Check remainder of intersection with block for other pair
          int z1, z2;
          blk_coords(blk_index(i, j), &z1, &z2);
          for (int k = j + 1; k < z2 + BLK_WIDTH; k++) {
            uint16_t pair = inter & cells[i][k];
            if (pair) {
              for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                  if (cells[a][b] && !(a == i && b == j) && !(a == i && b == k)) {
                    cells[a][b] &= ~pair;

                    if (!(cells[a][b] & (cells[a][b] - 1))) {
                      remove_candidate(a, b);
                      for (int x = 0; x < HOUSE_SZ; x++) {
                        opts_count[x] = 0;
                      }

                      for (int j2 = 0; j2 < HOUSE_SZ; j2++) {
                        for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                          opts_count[k2] += (cells[i][j2] >> k2) & 1;
                        }
                      }

                      // Make a bitvector of numbers that can only go in two cells
                      uint16_t pairs = 0;
                      for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                        if (opts_count[k] == 2) {
                          pairs |= 1 << k2;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Columns
  for (int j = 0; j < HOUSE_SZ; j++) {
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if (bit_count(pairs) >= 1) {
      for (int i = 0; i < HOUSE_SZ; i++) {
        uint16_t inter = cells[i][j] & pairs;
        if (inter) {
          int z1, z2;
          blk_coords(blk_index(i, j), &z1, &z2);
          for (int k = i + 1; k < z1 + BLK_WIDTH; k++) {
            uint16_t pair = inter & cells[k][j];
            if (pair) {
              for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                  if (cells[a][b] && !(a == i && b == j) && !(a == k && b == j)) {
                    cells[a][b] &= ~pair;
                    if (!(cells[a][b] & (cells[a][b] - 1))) {
                      remove_candidate(a, b);

                      for (int x = 0; x < HOUSE_SZ; x++) {
                        opts_count[x] = 0;
                      }

                      for (int i2 = 0; i2 < HOUSE_SZ; i2++) {
                        for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                          opts_count[k2] += (cells[i2][j] >> k2) & 1;
                        }
                      }

                      uint16_t pairs = 0;
                      for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                        if (opts_count[k2] == 2) {
                          pairs |= 1 << k2;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

// Pointing pairs strategy: Within a sqaure, find pairs of cells in the same
// row/column that are the only two that can have a number, eliminate this
// option from the row/column
static void pointing_pairs() {
  // Look for pointing pairs in each block
  for (int z = 0; z < HOUSE_SZ; z++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    int z1, z2;
    blk_coords(z, &z1, &z2);
    for (int a = z1; a < z1 + BLK_WIDTH; a++) {
      for (int b = z2; b < z2 + BLK_WIDTH; b++) {
        for (int k = 0; k < HOUSE_SZ; k++) {
          opts_count[k] += (cells[a][b] >> k) & 1;
        }
      }
    }

    // Make bitvector of numbers that can only go in two cells
    uint16_t pairs = 0;
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        pairs |= 1 << k;
      }
    }

    if (bit_count(pairs) > 0) {
      // Find the pairs of cells with these numbers
      for (int a = z1; a < z1 + BLK_WIDTH; a++) {
        for (int b = z2; b < z2 + BLK_WIDTH; b++) {

          // If cell contains a pair, check the row/column for the other
          uint16_t inter = cells[a][b] & pairs;
          if (inter) {

            // Column
            for (int y = a + 1; inter && y < z1 + BLK_WIDTH; y++) {
              uint16_t pair = inter & cells[y][b];
              if (pair) {
                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (cells[k][b] && k != y && k != a) {
                    cells[k][b] &= ~pair;
                    if (!(cells[k][b] & (cells[k][b] - 1))) {
                      remove_candidate(k, b);
                    }
                  }
                }
                inter &= ~pair; // Pair is found
              }
            }

            // Row
            for (int x = b + 1; inter && x < z2 + BLK_WIDTH; x++) {
              uint16_t pair = inter & cells[a][x];
              if (pair) {
                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (cells[a][k] && k != x && k != b) {
                    cells[a][k] &= ~pair;
                    if (!(cells[a][k] & (cells[a][k] - 1))) {
                      remove_candidate(a, k);
                    }
                  }
                }
                inter &= ~pair; // Pair is found
              }
            }
          }
        }
      }
    }
  }
}

// Pointing tuples strategy: same as pointing pairs, but is agnostic of 
// group size
static void pointing_tuples() {
  // Look for pointing tuples in each block
  for (int z = 0; z < HOUSE_SZ; z++) {
    int z1, z2;
    blk_coords(z, &z1, &z2);

    uint16_t rows[BLK_WIDTH];
    uint16_t cols[BLK_WIDTH];
    for (int i = 0; i < BLK_WIDTH; i++) {
      rows[i] = 0;
      cols[i] = 0;
    }

    for (int a = 0; a < BLK_WIDTH; a++) {
      for (int b = 0; b < BLK_WIDTH; b++) {
        rows[a] |= cells[z1 + a][z2 + b];
        cols[b] |= cells[z1 + a][z2 + b];
      }
    }

    // Columns
    for (int i = 0; i < BLK_WIDTH; i++) {
      for (int j = i + 1; j < BLK_WIDTH; j++) {
        uint16_t inter = rows[i] & rows[j];
        if (inter) {
          for (uint16_t vec = 1; vec < 1 << HOUSE_SZ; vec <<= 1) {
            if (vec & inter) {
              int count = 0;
              int col = 0;
              for (int k = 0; k < BLK_WIDTH; k++) {
                if (vec & cols[k]) {
                  vec &= cols[k];
                  col = k;
                  count++;
                }
              }
              if (count == 1) {
                for (int y = 0; y < HOUSE_SZ; y++) {
                  if ((y < z1 || y >= z1 + BLK_WIDTH) && cells[y][z2 + col]) {
                    cells[y][z2 + col] &= ~vec;
                    if (!(cells[y][z2 + col] & (cells[y][z2 + col] - 1))) {
                      remove_candidate(y, z2 + col);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    // Rows
    for (int j = 0; j < BLK_WIDTH; j++) {
      for (int i = j + 1; i < BLK_WIDTH; i++) {
        uint16_t inter = cols[i] & cols[j];
        if (inter) {
          for (uint16_t vec = 1; vec < 1 << HOUSE_SZ; vec <<= 1) {
            if (vec & inter) {
              int count = 0;
              int row = 0;
              for (int k = 0; k < BLK_WIDTH; k++) {
                if (vec & rows[k]) {
                  vec &= rows[k];
                  row = k;
                  count++;
                }
              }
              if (count == 1) {
                for (int x = 0; x < HOUSE_SZ; x++) {
                  if ((x < z2 || x >= z2 + BLK_WIDTH) && cells[z1 + row][x]) {
                    cells[z1 + row][x] &= ~vec;
                    if (!(cells[z1 + row][x] & (cells[z1 + row][x] - 1))) {
                      remove_candidate(z1 + row, x);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

// X-Wing strategy: An x-wing pattern is formed by two houses that have the same
// candidate pair in the same rows/columns. Eliminate candidate from rows/columns.
static void x_wing() {

  // Row
  // Build pair vectors for each row
  // Bit is set in vector if that number appears exactly twice in row
  uint16_t row_pairs[HOUSE_SZ];
  for (int x = 0; x < HOUSE_SZ; x++) {
    row_pairs[x] = 0;
  }

  for (int i = 0; i < HOUSE_SZ; i++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int j = 0; j < HOUSE_SZ; j++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make bitvector of numbers that can only go in two cells
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        row_pairs[i] |= 1 << k;
      }
    }
  }

  // Identify x-wings in rows
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = i + 1; j < HOUSE_SZ; j++) {
      uint16_t inter = row_pairs[i] & row_pairs[j];
      for (int n = 0; (inter >> n) != 0; n++) {
        if (!(inter & (1 << n)))
          continue;

        // Find columns of pair in row i
        int col1 = 0, col2 = 0; // There should only be two - they are pairs
        int found = 0;
        for (int k = 0; k < HOUSE_SZ; k++) {
          if (cells[i][k] & (1 << n)) {
            if (!found) {
              found = 1;
              col1 = k;
            } else {
              col2 = k;
            }
          }
        }

        // Do columns match in row j? If so, eliminate from columns
        if ((cells[j][col1] & (1 << n)) && (cells[j][col2] & (1 << n))) {
          for (int k = 0; k < HOUSE_SZ; k++) {
            if (k != i && k != j) {
              cells[k][col1] &= ~(1 << n);
              if (cells[k][col1] && !(cells[k][col1] & (cells[k][col1] - 1))) {
                remove_candidate(k, col1);

                for (int x = 0; x < HOUSE_SZ; x++) {
                  row_pairs[x] = 0;
                }

                for (int i2 = 0; i2 < HOUSE_SZ; i2++) {
                  int opts_count[HOUSE_SZ];
                  for (int x2 = 0; x2 < HOUSE_SZ; x2++) {
                    opts_count[x2] = 0;
                  }

                  for (int j2 = 0; j2 < HOUSE_SZ; j2++) {
                    for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                      opts_count[k2] += (cells[i2][j2] >> k2) & 1;
                    }
                  }

                  for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                    if (opts_count[k2] == 2) {
                      row_pairs[i2] |= 1 << k2;
                    }
                  }
                }
              }

              cells[k][col2] &= ~(1 << n);
              if (cells[k][col2] && !(cells[k][col2] & (cells[k][col2] - 1))) {
                remove_candidate(k, col2);

                for (int x = 0; x < HOUSE_SZ; x++) {
                  row_pairs[x] = 0;
                }

                for (int i2 = 0; i2 < HOUSE_SZ; i2++) {
                  int opts_count[HOUSE_SZ];
                  for (int x = 0; x < HOUSE_SZ; x++) {
                    opts_count[x] = 0;
                  }

                  for (int j2 = 0; j2 < HOUSE_SZ; j2++) {
                    for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                      opts_count[k2] += (cells[i2][j2] >> k2) & 1;
                    }
                  }

                  for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                    if (opts_count[k2] == 2) {
                      row_pairs[i2] |= 1 << k2;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Column
  // Build pair vectors for each column
  uint16_t col_pairs[HOUSE_SZ];
  for (int x = 0; x < HOUSE_SZ; x++) {
    col_pairs[x] = 0;
  }

  for (int j = 0; j < HOUSE_SZ; j++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    for (int i = 0; i < HOUSE_SZ; i++) {
      for (int k = 0; k < HOUSE_SZ; k++) {
        opts_count[k] += (cells[i][j] >> k) & 1;
      }
    }

    // Make bitvector of numbers that can only go in two cells
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        col_pairs[j] |= 1 << k;
      }
    }
  }

  // Identify x-wings in columns
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = i + 1; j < HOUSE_SZ; j++) {
      uint16_t inter = col_pairs[i] & col_pairs[j];
      for (int n = 0; (inter >> n) != 0; n++) {
        if (!(inter & (1 << n)))
          continue;

        // Find rows of pair in column i
        int row1 = 0, row2 = 0; // There should only be two - they are pairs
        int found = 0;
        for (int k = 0; k < HOUSE_SZ; k++) {
          if (cells[k][i] & (1 << n)) {
            if (!found) {
              found = 1;
              row1 = k;
            } else {
              row2 = k;
            }
          }
        }

        // Do rows match in column j? If so, eliminate from rows
        if ((cells[row1][j] & (1 << n)) && (cells[row2][j] & (1 << n))) {
          for (int k = 0; k < HOUSE_SZ; k++) {
            if (k != i && k != j) {
              cells[row1][k] &= ~(1 << n);
              if (cells[row1][k] && !(cells[row1][k] & (cells[row1][k] - 1))) {
                remove_candidate(row1, k);

                for (int x = 0; x < HOUSE_SZ; x++) {
                  col_pairs[x] = 0;
                }

                for (int j2 = 0; j2 < HOUSE_SZ; j2++) {
                  int opts_count[HOUSE_SZ];
                  for (int x = 0; x < HOUSE_SZ; x++) {
                    opts_count[x] = 0;
                  }

                  for (int i2 = 0; i2 < HOUSE_SZ; i2++) {
                    for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                      opts_count[k2] += (cells[i2][j2] >> k2) & 1;
                    }
                  }

                  for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                    if (opts_count[k2] == 2) {
                      col_pairs[j2] |= 1 << k2;
                    }
                  }
                }
              }

              cells[row2][k] &= ~(1 << n);
              if (cells[row2][k] && !(cells[row2][k] & (cells[row2][k] - 1))) {
                remove_candidate(row2, k);

                for (int x = 0; x < HOUSE_SZ; x++) {
                  col_pairs[x] = 0;
                }

                for (int j2 = 0; j2 < HOUSE_SZ; j2++) {
                  int opts_count[HOUSE_SZ];
                  for (int x = 0; x < HOUSE_SZ; x++) {
                    opts_count[x] = 0;
                  }

                  for (int i2 = 0; i2 < HOUSE_SZ; i2++) {
                    for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                      opts_count[k2] += (cells[i2][j2] >> k2) & 1;
                    }
                  }

                  for (int k2 = 0; k2 < HOUSE_SZ; k2++) {
                    if (opts_count[k2] == 2) {
                      col_pairs[j] |= 1 << k2;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Build pair vectors for each sqaure
  uint16_t blk_pairs[HOUSE_SZ];
  for (int x = 0; x < HOUSE_SZ; x++) {
    blk_pairs[x] = 0;
  }

  for (int z = 0; z < HOUSE_SZ; z++) {
    // Count number of cells that can hold each number
    int opts_count[HOUSE_SZ];
    for (int x = 0; x < HOUSE_SZ; x++) {
      opts_count[x] = 0;
    }

    int z1, z2;
    blk_coords(z, &z1, &z2);
    for (int a = z1; a < z1 + BLK_WIDTH; a++) {
      for (int b = z2; b < z2 + BLK_WIDTH; b++) {
        for (int k = 0; k < HOUSE_SZ; k++) {
          opts_count[k] += (cells[a][b] >> k) & 1;
        }
      }
    }

    // Make bitvector of numbers that can only go in two cells
    for (int k = 0; k < HOUSE_SZ; k++) {
      if (opts_count[k] == 2) {
        blk_pairs[z] |= 1 << k;
      }
    }
  }

  // Identify x-wings between blocks
  for (int i = 0; i < HOUSE_SZ; i++) {

    // Compare horizontally
    for (int j = i + 1; j < (i / BLK_WIDTH) * BLK_WIDTH + BLK_WIDTH; j++) {
      uint16_t inter = blk_pairs[i] & blk_pairs[j];
      for (int n = 0; (inter >> n) != 0; n++) {
        if (!(inter & (1 << n)))
          continue;

        int row1 = 0, row2 = 0;
        int found = 0;
        int z1, z2;
        blk_coords(i, &z1, &z2);
        for (int a = z1; a < z1 + BLK_WIDTH; a++) {
          for (int b = z2; b < z2 + BLK_WIDTH; b++) {
            if (cells[a][b] & (1 << n)) {
              if (!found) {
                found = 1;
                row1 = a;
              } else {
                row2 = a;
              }
            }
          }
        }

        // This is a pointing pair, not an x-wing pattern
        if (row1 == row2)
          continue;

        // Do rows match in block j? If so, eliminate from third block
        blk_coords(j, &z1, &z2);
        uint16_t row1_or = cells[row1][z2] | cells[row1][z2 + 1] | cells[row1][z2 + 2];
        uint16_t row2_or = cells[row2][z2] | cells[row2][z2 + 1] | cells[row2][z2 + 2];

        if ((row1_or & (1 << n)) && (row2_or & (1 << n))) {
          int elim_blk = 0;
          if (i % BLK_WIDTH)                             // 1,2
            elim_blk = i - 1;
          else if (i % BLK_WIDTH == 0 && j % BLK_WIDTH == 1)  // 0,1
            elim_blk = j + 1;
          else                                      // 0,2
            elim_blk = i + 1;

          blk_coords(elim_blk, &z1, &z2);
          for (int b = z2; b < z2 + BLK_WIDTH; b++) {
            cells[row1][b] &= ~(1 << n);
            if (cells[row1][b] && !(cells[row1][b] & (cells[row1][b] - 1))) {
              remove_candidate(row1, b);

              for (int x = 0; x < HOUSE_SZ; x++) {
                blk_pairs[x] = 0;
              }

              for (int z = 0; z < HOUSE_SZ; z++) {
                int opts_count[HOUSE_SZ];
                for (int x = 0; x < HOUSE_SZ; x++) {
                  opts_count[x] = 0;
                }

                for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                  for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                    for (int k = 0; k < HOUSE_SZ; k++) {
                      opts_count[k] += (cells[a][b] >> k) & 1;
                    }
                  }
                }

                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (opts_count[k] == 2) {
                    blk_pairs[z] |= 1 << k;
                  }
                }
              }
            }

            cells[row2][b] &= ~(1 << n);
            if (cells[row2][b] && !(cells[row2][b] & (cells[row2][b] - 1))) {
              remove_candidate(row2, b);

              for (int x = 0; x < HOUSE_SZ; x++) {
                blk_pairs[x] = 0;
              }

              for (int z = 0; z < HOUSE_SZ; z++) {
                int opts_count[HOUSE_SZ];
                for (int x = 0; x < HOUSE_SZ; x++) {
                  opts_count[x] = 0;
                }

                for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                  for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                    for (int k = 0; k < HOUSE_SZ; k++) {
                      opts_count[k] += (cells[a][b] >> k) & 1;
                    }
                  }
                }

                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (opts_count[k] == 2) {
                    blk_pairs[z] |= 1 << k;
                  }
                }
              }
            }
          }
        }
      }
    }

    // Compare vertically
    for (int j = i + BLK_WIDTH; j < HOUSE_SZ; j += BLK_WIDTH) {
      uint16_t inter = blk_pairs[i] & blk_pairs[j];
      for (int n = 0; (inter >> n) != 0; n++) {
        if (!(inter & (1 << n)))
          continue;

        int col1 = 0, col2 = 0;
        int found = 0;
        int z1, z2;
        blk_coords(i, &z1, &z2);
        for (int a = z1; a < z1 + BLK_WIDTH; a++) {
          for (int b = z2; b < z2 + BLK_WIDTH; b++) {
            if (cells[a][b] & (1 << n)) {
              if (!found) {
                found = 1;
                col1 = b;
              } else {
                col2 = b;
              }
            }
          }
        }

        // This is a pointing pair, not an x-wing pattern
        if (col1 == col2)
          continue;

        // Do columns match in block j? If so, eliminate from third block
        blk_coords(j, &z1, &z2);
        uint16_t col1_or = cells[z1][col1] | cells[z1 + 1][col1] | cells[z1 + 2][col1];
        uint16_t col2_or = cells[z1][col2] | cells[z1 + 1][col2] | cells[z1 + 2][col2];

        if ((col1_or & (1 << n)) && (col2_or & (1 << n))) {
          int elim_blk = 0;
          if (i / BLK_WIDTH)
            elim_blk = i - BLK_WIDTH;
          else if (i / BLK_WIDTH == 0 && j / BLK_WIDTH == 1)
            elim_blk = j + BLK_WIDTH;
          else
            elim_blk = i + BLK_WIDTH;

          blk_coords(elim_blk, &z1, &z2);
          for (int a = z1; a < z1 + BLK_WIDTH; a++) {
            cells[a][col1] &= ~(1 << n);
            if (cells[a][col1] && !(cells[a][col1] & (cells[a][col1] - 1))) {
              remove_candidate(a, col1);

              for (int x = 0; x < HOUSE_SZ; x++) {
                blk_pairs[x] = 0;
              }

              for (int z = 0; z < HOUSE_SZ; z++) {
                int opts_count[HOUSE_SZ];
                for (int x = 0; x < HOUSE_SZ; x++) {
                  opts_count[x] = 0;
                }

                for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                  for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                    for (int k = 0; k < HOUSE_SZ; k++) {
                      opts_count[k] += (cells[a][b] >> k) & 1;
                    }
                  }
                }

                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (opts_count[k] == 2) {
                    blk_pairs[z] |= 1 << k;
                  }
                }
              }
            }

            cells[a][col2] &= ~(1 << n);
            if (cells[a][col2] && !(cells[a][col2] & (cells[a][col2] - 1))) {
              remove_candidate(a, col2);

              for (int x = 0; x < HOUSE_SZ; x++) {
                blk_pairs[x] = 0;
              }

              for (int z = 0; z < HOUSE_SZ; z++) {
                int opts_count[HOUSE_SZ];
                for (int x = 0; x < HOUSE_SZ; x++) {
                  opts_count[x] = 0;
                }

                for (int a = z1; a < z1 + BLK_WIDTH; a++) {
                  for (int b = z2; b < z2 + BLK_WIDTH; b++) {
                    for (int k = 0; k < HOUSE_SZ; k++) {
                      opts_count[k] += (cells[a][b] >> k) & 1;
                    }
                  }
                }

                for (int k = 0; k < HOUSE_SZ; k++) {
                  if (opts_count[k] == 2) {
                    blk_pairs[z] |= 1 << k;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

// Naked triplets strategy: same as pairs, but must be a group of three cells
static void naked_triplets() {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      // If cell solved, continue
      if (!cells[i][j])
        continue;

      // Row
      for (int x1 = j + 1; x1 < HOUSE_SZ; x1++) {
        if (!cells[i][x1])
          continue;

        for (int x2 = x1 + 1; x2 < HOUSE_SZ; x2++) {
          if (!cells[i][x2])
            continue;

          uint16_t un = cells[i][j] | cells[i][x1] | cells[i][x2];
          if (bit_count(un) == 2) {
            for(int k = 0; k < HOUSE_SZ; k++) {
              if (cells[i][k] && k != j && k != x1 && k != x2) {
                cells[i][k] &= ~un;
                if (!(cells[i][k] & (cells[i][k] - 1))) {
                  remove_candidate(i, k);
                }
              }
            }
            // XXX Way to jump to next section (column)? There won't be another triplet
          }
        }
      }

      // Column - XXX Don't need to iterate all the way to end of column
      for (int y1 = i + 1; y1 < HOUSE_SZ; y1++) {
        if (!cells[y1][j])
          continue;

        for (int y2 = y1 + 1; y2 < HOUSE_SZ; y2++) {
          if (!cells[y2][j])
            continue;

          uint16_t un = cells[i][j] | cells[y1][j] | cells[y2][j];
          if (bit_count(un) == 3) {
            for (int k = 0; k < HOUSE_SZ; k++) {
              if (cells[k][j] && k != i && k != y1 && k != y2) {
                cells[k][j] &= ~un;
                if (!(cells[k][j] & (cells[k][j] - 1))) {
                  remove_candidate(k, j);
                }
              }
            }
          }
        }
      }

      // Block
      int z1, z2;
      blk_coords(blk_index(i, j), &z1, &z2);

      for (int a = i; a < z1 + BLK_WIDTH; a++) {
        for (int b = z2; b < z2 + BLK_WIDTH; b++) {
          if (a == i && b <= j)
            continue;

          if (!cells[a][b])
            continue;

          for (int c = a; c < z1 + BLK_WIDTH; c++) {
            for (int d = z2; d < z2 + BLK_WIDTH; d++) {
              if (c == a && d <= b)
                continue;

              if (!cells[c][d])
                continue;

              uint16_t un = cells[i][j] | cells[a][b] | cells[c][d];
              if (bit_count(un) == 3) {
                for (int k = z1; k < z1 + BLK_WIDTH; k++) {
                  for (int l = z2; l < z2 + BLK_WIDTH; l++) {
                    if (cells[k][l] && !((k == i && l == j) || (k == a && l == b) ||
                          (k == c && l == d))) {
                      cells[k][l] &= ~un;
                      if (!(cells[k][l] & (cells[k][k] - 1))) {
                        remove_candidate(k, l);
                      }
                    }
                  }
                }
              }
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

  // Initialize grids
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      cells[i][j] = (1 << HOUSE_SZ) - 1;
      solved[i][j] = 0;
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
          remove_candidate(i, j);
        } else {
          fprintf(stderr, "Invalid digit: %d\n", d);
          return 1;
        }
      }
    }
  }

  // Cross off initial round of bits
  uint16_t rowfin[HOUSE_SZ];
  uint16_t colfin[HOUSE_SZ];
  uint16_t blkfin[HOUSE_SZ];

  memset(&rowfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&colfin, 0, HOUSE_SZ * sizeof(uint16_t));
  memset(&blkfin, 0, HOUSE_SZ * sizeof(uint16_t));

  update_solved(rowfin, colfin, blkfin);

  for (int i = 0; i < 15; i++) {
    singles();
    pointing_tuples();

    update_solved(rowfin, colfin, blkfin);
    if (is_solved(rowfin, colfin, blkfin)) {
      printf("Solved in %d iterations\n", i);
      return 0;
    }
  }

  printf("Not solved\n");

  return 1;
}

/* vim:set ts=2 sw=2 et: */
