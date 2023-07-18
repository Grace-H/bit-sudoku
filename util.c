/**
 * util.c
 * Utlity functions for sudoku solver
 *
 * @author Grace-H
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n",       \
    __func__, __LINE__, ##__VA_ARGS__)


// Count number of set bits in vector of length GRP_SZ
int bit_count(const uint16_t n) {
  int count = 0;
  for (int i = 0; i < GRP_SZ; i++) {
    count += (n >> i) & 1;
  }
  return count;
}

int cells_str(uint16_t cells[GRP_SZ][GRP_SZ], char *buf, int n) {
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

int vec_str(const uint16_t vec, char *buf, int n) {
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

void stack_push(struct stack *stack, void *datum) {
	struct node *node = malloc(sizeof(struct node));
	node->datum = datum;
	node->next = stack->head;
	stack->head = node;
};

void *stack_pop(struct stack *stack) {
	if (!stack->head)
		return NULL;

	struct node *node = stack->head;
	stack->head = node->next;
	void *datum = node->datum;
	free(node);
	return datum;
}

int stack_is_empty(struct stack *stack) {
	return stack->head == NULL;
}

/* vim:set ts=2 sw=2 et: */
