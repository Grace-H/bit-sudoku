/**
 * util.h
 *
 * @author: Grace-H
 */

#include <stdint.h>

#define HOUSE_SZ 9 // Cells in one house
#define BLK_WIDTH 3 // Cells in intersection between houses & width of one block
#define N_CELLS 81

#define LOG(format, ...) fprintf(stderr, "%s(%d):\t" format "\n",       \
    __func__, __LINE__, ##__VA_ARGS__)

// Grid & vector operations
int bit_count(const uint16_t n);
void copy_cells(uint16_t src[HOUSE_SZ][HOUSE_SZ], uint16_t dst[HOUSE_SZ][HOUSE_SZ]);

// toString functions
int cells_str(uint16_t cells[HOUSE_SZ][HOUSE_SZ], char *buf, int n);
int vec_str(const uint16_t vec, char *buf, int n);

// Nodes for data structures
struct node {
	struct node *next;
	void *datum;
};

// Stack
struct stack {
	struct node *head;
};

void stack_init(struct stack *stack);
void stack_destroy(struct stack *stack);

void stack_push(struct stack *stack, void *datum);
void *stack_pop(struct stack *stack);
int stack_is_empty(struct stack *stack);

// Queue
struct queue {
  struct node *head;
  struct node *tail;
};

void queue_init(struct queue *queue);
void queue_destroy(struct queue *queue);

void queue_put(struct queue *queue, void *datum);
void *queue_get(struct queue *queue);
int queue_is_empty(struct queue *queue);

// Priority Queue
struct pq {
  int (*priority)(void *);
  void **array;
  int size;
  int array_n;
};

void pq_init(struct pq *pq, int (*priority)(void *), int size);
void pq_destroy(struct pq *pq);

int pq_insert(struct pq *pq, void *datum);
void *pq_extract_max(struct pq *pq);
void pq_change_key(struct pq *pq, void *datum);
int pq_is_empty(struct pq *pq);

/* vim:set ts=2 sw=2 et: */
