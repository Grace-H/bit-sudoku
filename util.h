/**
 * util.h
 *
 * @author: Grace-H
 */

#include <stdint.h>

#define GRP_SZ 9
#define CELL 3

int bit_count(const uint16_t n);

int cells_str(uint16_t cells[GRP_SZ][GRP_SZ], char *buf, int n);
int vec_str(const uint16_t vec, char *buf, int n);

