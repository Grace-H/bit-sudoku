#include <stdio.h>
#include <assert.h>
#include "../util.h"

struct pq pq;
void beforeEach() {
	pq_init(&pq);
}

void afterEach() {
	pq_destroy(&pq);
}

void test_is_empty() {
	assert(pq_is_empty(&pq));
	int datum = 71;
	pq_insert(&pq, &datum, 1);
	assert(!pq_is_empty(&pq));
}

void test_insert_low2high() {
	int priorities[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
}

void test_insert_high2low() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}
}

void test_extract_insert() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	for (int i = 0; i < 5; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}

	while (!pq_is_empty(&pq)) {
		pq_extract_max(&pq);
	}

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}
		
	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		assert(*temp <= *max);
		max = temp;
	}

}

void test_alternate() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	int *max = NULL;
	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
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
}


void test_increase_key() {
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int priorities[10] = {0, 1, 2, 3, 9, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &data[i], priorities[i]);
	}

	pq_change_key(&pq, &data[4], 4);

	int *max = NULL;
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (max)
			assert(*temp <= *max);
		max = temp;
	}
}

void test_decrease_key() {
	int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	int priorities[10] = {0, 1, 2, 3, 0, 5, 6, 7, 8, 9};

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &data[i], priorities[i]);
	}

	pq_change_key(&pq, &data[4], 4);

	int *max = NULL;
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (max)
			assert(*temp <= *max);
		max = temp;
	}
}
