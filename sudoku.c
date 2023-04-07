/* sudoku.c
 * Sudoku solver, representing cell possibilities as a bitvector
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ROWLEN 9

#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n", \
				 __func__, __LINE__, ##__VA_ARGS__)

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE\n", argv[0]);
    return 1;
  }

  // Initialize bitvectors of cell possibilites
  uint16_t cells[ROWLEN][ROWLEN];
  for (int i = 0; i < ROWLEN; i++) {
    for (int j = 0; j < ROWLEN; j++) {
      cells[i][j] = (1 << ROWLEN) - 1;
    }
  }

  // Read & parse puzzle from file
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    perror("open");
    return 1;
  }

  char buf[ROWLEN];
  for (int i = 0; i < ROWLEN; i++) {
    int ret = fscanf(f, "%9c\n", buf);
    if (ret < 1) {
      perror("fgets");
      return 1;
    }
    
    for (int j = 0; j < ROWLEN; j++) {
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
  return 0;
}
