#include <stdio.h>
#include <assert.h>
#include "../util.h"

struct queue queue;

void test_is_empty() {
	queue_init(&queue);
	assert(queue_is_empty(&queue));
	int datum = 71;
	queue_put(&queue, &datum);
	assert(!queue_is_empty(&queue));
	queue_destroy(&queue);
}

void test_spurious_remove() {
	queue_init(&queue);
	void *datum = queue_get(&queue);
	assert(datum == NULL);
	queue_destroy(&queue);
}

void test_get_remove() {
	queue_init(&queue);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		queue_put(&queue, &data[i]);
	}

	for (int i = 0; i < 10; i++) {
		assert(queue_get(&queue) == &data[i]);
	}

	queue_destroy(&queue);
}

void test_empty_get() {
	queue_init(&queue);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 5; i++) {
		queue_put(&queue, &data[i]);
	}

	while (!queue_is_empty(&queue)) {
		queue_get(&queue);
	}
	assert(queue_is_empty(&queue));

	for (int i = 0; i < 10; i++) {
		queue_put(&queue, &data[i]);
	}
		
	int *max = queue_get(&queue);
	while (!queue_is_empty(&queue)) {
		int *temp = queue_get(&queue);
		assert(*temp >= *max);
		max = temp;
	}

	queue_destroy(&queue);
}

void test_alternate() {
	queue_init(&queue);
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		queue_put(&queue, &data[i]);
		if (i % 3 == 0) {
			assert(queue_get(&queue) == &data[i / 3]);
		}
	}

	queue_destroy(&queue);
}
