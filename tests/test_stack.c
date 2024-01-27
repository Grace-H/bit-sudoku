#include <stdio.h>
#include <assert.h>
#include "../util.h"

struct stack stack;

void test_is_empty() {
	stack_init(&stack);
	assert(stack_is_empty(&stack));
	int datum = 71;
	stack_push(&stack, &datum);
	assert(!stack_is_empty(&stack));
	stack_destroy(&stack);
}

void test_spurious_pop() {
	stack_init(&stack);
	void *datum = stack_pop(&stack);
	assert(datum == NULL);
	stack_destroy(&stack);
}

void test_push_pop() {
	stack_init(&stack);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		stack_push(&stack, &data[i]);
	}

	for (int i = 9; i >= 0; i--) {
		assert(stack_pop(&stack) == &data[i]);
	}

	stack_destroy(&stack);
}

void test_empty_pop() {
	stack_init(&stack);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 5; i++) {
		stack_push(&stack, &data[i]);
	}

	while (!stack_is_empty(&stack)) {
		stack_pop(&stack);
	}
	assert(stack_is_empty(&stack));

	for (int i = 0; i < 10; i++) {
		stack_push(&stack, &data[i]);
	}
		
	int *prev = stack_pop(&stack);
	while (!stack_is_empty(&stack)) {
		int *temp = stack_pop(&stack);
		assert(*temp <= *prev);
		prev = temp;
	}

	stack_destroy(&stack);
}

void test_alternate() {
	stack_init(&stack);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		stack_push(&stack, &data[i]);
		if (i % 3 == 0) {
			assert(stack_pop(&stack) == &data[i]);
		}
	}

	stack_destroy(&stack);
}
