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

// Count number of set bits in vector of length HOUSE_SZ
int bit_count(const uint16_t n) {
  int count = 0;
  for (int i = 0; i < HOUSE_SZ; i++) {
    count += (n >> i) & 1;
  }
  return count;
}

void copy_cells(uint16_t src[HOUSE_SZ][HOUSE_SZ], uint16_t dst[HOUSE_SZ][HOUSE_SZ]) {
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ; j++) {
      dst[i][j] = src[i][j];
    }
  }
}

int cells_str(uint16_t cells[HOUSE_SZ][HOUSE_SZ], char *buf, int n) {
  if (n < (HOUSE_SZ + 1) * HOUSE_SZ) {
    LOG("too short");
    errno = EINVAL;
    return -1;
  }

  int k = 0;
  for (int i = 0; i < HOUSE_SZ; i++) {
    for (int j = 0; j < HOUSE_SZ + 1; j++) {
      if (j == HOUSE_SZ) {
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
  if (n < HOUSE_SZ + 1) {
    errno = EINVAL;
    return -1;
  }

  int i;
  for (i = 0; i < HOUSE_SZ; i++) {
    buf[i] = ((vec >> (HOUSE_SZ - i - 1)) & 1) + '0';
  }

  buf[i] = '\0';
  return i;
}

void stack_init(struct stack *stack) {
  stack->head = NULL;
}

void stack_destroy(struct stack *stack) {
  while (stack->head) {
    struct node *node = stack->head;
    stack->head = node->next;
    free(node);
  }
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

void queue_init(struct queue *queue) {
  queue->head = NULL;
  queue->tail = NULL;
}
void queue_destroy(struct queue *queue) {
  while(queue->head) {
    struct node *node = queue->head;
    queue->head = node->next;
    free(node);
  }
  queue->tail = NULL;
}

void queue_put(struct queue *queue, void *datum) {
  struct node *node = malloc(sizeof(struct node));
  node->datum = datum;
  node->next = NULL;

  if (queue->head)
    queue->tail->next = node;
  else
    queue->head = node;

  queue->tail = node;
}

void *queue_get(struct queue *queue) {
  if (!queue->head)
    return NULL;

  struct node *node = queue->head;
  queue->head = node->next;
  if (!queue->head)
    queue->tail = NULL;
  void *datum = node->datum;
  free(node);
  return datum;
}

int queue_is_empty(struct queue *queue) {
  return queue->head == NULL;
}

void pq_init(struct pq *pq, int (*priority)(void *), int size) {
  pq->priority = priority;
  pq->size = 0;
  pq->array = malloc(sizeof(void *) * size);
  pq->array_n = size;
}

void pq_destroy(struct pq *pq) {
  free(pq->array);
  pq->array = NULL;
  pq->priority = NULL;
  pq->size = 0;
  pq->array_n = 0;
}

static void pq_change_key_at(struct pq *pq, int i) {
  if (i != 0 && pq->priority(pq->array[i]) > pq->priority(pq->array[(i - 1) / 2])) {
    // Larger than parent (increase key)
    while (pq->priority(pq->array[i]) > pq->priority(pq->array[(i - 1) / 2])) {
      void *temp = pq->array[i];
      pq->array[i] = pq->array[(i - 1) / 2];
      pq->array[(i - 1) / 2] = temp;
      i = (i - 1) / 2;
    }
  } else {
    // Smaller than child (decrease key)
    while (i * 2 + 1 < pq->size) {
      int lchild = pq->priority(pq->array[i * 2 + 1]);
      int rchild = i * 2 + 2 < pq->size ? pq->priority(pq->array[i * 2 + 2]) : i;
      int self = pq->priority(pq->array[i]);
      int to_swap = i;

      if (self < lchild) {
        if (self < rchild && rchild > lchild) {
          to_swap = i * 2 + 2;
        } else {
          to_swap = i * 2 + 1;
        }
      } else if (self < rchild) {
        to_swap = i * 2 + 2;
      }

      if (to_swap == i)
        break;

      void *temp = pq->array[i];
      pq->array[i] = pq->array[to_swap];
      pq->array[to_swap] = temp;
      i = to_swap;
    }
  }
}

int pq_insert(struct pq *pq, void *datum) {
  if (pq->size == pq->array_n)
    return 1;

  int i = pq->size;
  pq->array[i] = datum;

  pq->size++;
  pq_change_key_at(pq, i);

  return 0;
}

void *pq_extract_max(struct pq *pq) {
  void *to_return = pq->array[0];
  pq->array[0] = pq->array[pq->size - 1];
  pq->size--;
  pq_change_key_at(pq, 0);
  return to_return;
}


void pq_change_key(struct pq *pq, void *datum) {
  int i = 0;
  for (; i < pq->size; i++) {
    if (pq->array[i] == datum)
      break;
  }

  if (i == pq->size)
    return;

  pq_change_key_at(pq, i);
}

int pq_is_empty(struct pq *pq) {
  return pq->size == 0;
}

/* vim:set ts=2 sw=2 et: */
