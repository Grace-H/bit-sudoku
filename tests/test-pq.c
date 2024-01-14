#include <stdio.h>
#include "../util.h"

int test_create_destroy() {
	struct pq pq;
	pq_init(&pq);
	if (!pq_is_empty(&pq))
		goto out_fail;
	pq_destroy(&pq);
	return 0;
out_fail:
	pq_destroy(&pq);
	return 1;
}

int test_insert_low2high() {
	int priorities[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

	struct pq pq;
	pq_init(&pq);

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (*temp > *max)
			goto out_fail;
		max = temp;
	}
		
	pq_destroy(&pq);

	return 0;
out_fail:
	pq_destroy(&pq);
	return 1;
}

int test_insert_high2low() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	struct pq pq;
	pq_init(&pq);

	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
	}

	int *max = pq_extract_max(&pq);
	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (*temp > *max)
			goto out_fail;
		max = temp;
	}
		
	pq_destroy(&pq);

	return 0;
out_fail:
	pq_destroy(&pq);
	return 1;
}

int test_extract_insert() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	struct pq pq;
	pq_init(&pq);

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
		if (*temp > *max)
			goto out_fail;
		max = temp;
	}

	pq_destroy(&pq);

	return 0;
out_fail:
	pq_destroy(&pq);
	return 1;
	
}

int test_alternate() {
	int priorities[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

	struct pq pq;
	pq_init(&pq);

	int *max = NULL;
	for (int i = 0; i < 10; i++) {
		pq_insert(&pq, &priorities[i], priorities[i]);
		if (i % 3) {
			int *temp = pq_extract_max(&pq);
			if (max && *temp > *max)
				goto out_fail;
			max = temp;
		}
	}

	while (!pq_is_empty(&pq)) {
		int *temp = pq_extract_max(&pq);
		if (*temp > *max)
			goto out_fail;
		max = temp;
	}

	pq_destroy(&pq);

	return 0;
out_fail:
	pq_destroy(&pq);
	return 1;
}

int main() {
	int (*test_cases[5])(void) = { test_create_destroy, 
		test_insert_high2low, test_insert_low2high, 
		test_extract_insert, test_alternate };

	int fails = 0;
	for (int i = 0; i < 5; i++) {
		fails += test_cases[i]();
	}

	printf("Tests failed: %d/%d\n", fails, 5);

	return 0;
}
