/* sudoku.c
 * Sudoku solver, representing cell possibilities as a bitvector
 */

#include <stdio.h>
#include <stdint.h>

#define CELL_SZ 9

#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n", \
				 __func__, __LINE__, ##__VA_ARGS__)

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }

  // Initialize bitvectors of cell possibilites
  uint16_t cells[CELL_SZ][CELL_SZ];
  for (int i = 0; i < CELL_SZ; i++) {
    for (int j = 0; j < CELL_SZ; j++) {
      cells[i][j] = (1 << CELL_SZ) - 1;
      LOG("%d", cells[i][j]);
    }
  }
  
  return 0;
}
