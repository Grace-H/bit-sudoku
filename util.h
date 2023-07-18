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
