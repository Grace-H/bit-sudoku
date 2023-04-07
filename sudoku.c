/* sudoku.c
 * Sudoku solver, representing cell possibilities as a bitvector
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ROW 9
#define CELL 3
#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n", \
				 __func__, __LINE__, ##__VA_ARGS__)

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }

  // Initialize bitvectors of cell possibilites
  uint16_t cells[ROW][ROW];
  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
      cells[i][j] = (1 << ROW) - 1;
    }
  }

  // Read & parse puzzle from file
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("open");
    return 1;
  }

  char buf[ROW];
  for (int i = 0; i < ROW; i++) {
    int ret = fscanf(f, "%9c\n", buf);
    if (ret < 1) {
      perror("fgets");
      return 1;
    }

    for (int j = 0; j < ROW; j++) {
      if (buf[j] != ' ') {
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
  uint16_t rowfin[ROW];
  uint16_t colfin[ROW];
  uint16_t sqrfin[ROW];
  for (int i = 0; i < ROW; i++) {
    rowfin[i] = 0;
    colfin[i] = 0;
    sqrfin[i] = 0;
  }

  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
      uint16_t c = cells[i][j];
      if (!(c & (c - 1))) {
	rowfin[i] |= c;
	colfin[j] |= c;
	sqrfin[i % 3 + ((j % 3) * 3)] |= c;
      }
    }
  }

  for (int i = 0; i < ROW; i++) {
    for (int j = 0; j < ROW; j++) {
      cells[i][j] ^= rowfin[i] ^ colfin[j] ^ sqrfin[i % 3 + ((j % 3) * 3)];
    }
  }
  return 0;
}
