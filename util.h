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

int bit_count(const uint16_t n);
void copy_cells(uint16_t src[HOUSE_SZ][HOUSE_SZ], uint16_t dst[HOUSE_SZ][HOUSE_SZ]);

int cells_str(uint16_t cells[HOUSE_SZ][HOUSE_SZ], char *buf, int n);
int vec_str(const uint16_t vec, char *buf, int n);

struct node {
	struct node *next;
	void *datum;
};

struct stack {
	struct node *head;
};

void stack_init(struct stack *stack);
void stack_destroy(struct stack *stack);

void stack_push(struct stack *stack, void *datum);
void *stack_pop(struct stack *stack);
int stack_is_empty(struct stack *stack);

/* vim:set ts=2 sw=2 et: */
