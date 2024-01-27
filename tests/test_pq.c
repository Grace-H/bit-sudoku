#include <stdio.h>
#include <assert.h>
#include "../util.h"

struct pq pq;

// Function that gives priority of datum; in this case the int pointed to
int priority(int *datum) {
	return *datum;
}

void test_is_empty() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	assert(pq_is_empty(&pq));
	int datum = 71;
	pq_insert(&pq, &datum);
	assert(!pq_is_empty(&pq));
	pq_destroy(&pq);
}

void test_insert_low2high() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}

void test_insert_high2low() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}

void test_extract_insert() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	for (int i = 0; i < 5; i++) {
		pq_insert(&pq, &priorities[i]);
	}

	while (!pq_is_empty(&pq)) {
		pq_extract_max(&pq);
	}

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
	}
		
	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}

void test_alternate() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	int *max = NULL;
	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
		if (i % 3) {
			int *temp = pq_extract_max(&pq);
			if (max)
			       assert(*temp <= *max);
			max = temp;
		}
	}

	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}


void test_decrease_key() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {0, 1, 2, 3, 9, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
	}

	priorities[4] = 4;
	pq_change_key(&pq, &priorities[4]);

	int *max = NULL;
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (max)
			assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}

void test_increase_key() {
	pq_init(&pq, (int (*)(void *)) priority, 10);
	int priorities[10] = {0, 1, 2, 3, 0, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i]);
	}

	pq_change_key(&pq, &priorities[4]);

	int *max = NULL;
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (max)
			assert(*temp <= *max);
		max = temp;
	}
	pq_destroy(&pq);
}
