#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

size_t triangle_len(size_t n) {
	return ((1 + n) * n) / 2;
}
size_t triangle_idx(size_t i, size_t j) {
	if (i < j) {
		size_t t = i;
		i = j;
		j = t;
	}

	size_t idx = ((i + 1) * i) / 2 + j;
	return idx;
}

int main() {
	size_t n = 5;
	int *triangle_arr = (int *)calloc(triangle_len(5), sizeof(int));
	if (!triangle_arr) {
		return EXIT_FAILURE;
	}

	int counter = 0;
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			size_t idx = triangle_idx(i, j);
			printf("[%zu][%zu] = %zu\n", i, j, idx);

			triangle_arr[idx] = counter++;
		}
		printf("\n");
	}


	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			size_t idx = triangle_idx(i, j);

			printf("%2d ", triangle_arr[idx]);
		}
		printf("\n");
	}

	printf("\n\n");

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j <= i; j++) {
			size_t idx = triangle_idx(i, j);

			printf("%2d ", triangle_arr[idx]);
		}
		printf("\n");
	}

	printf("\n\n");

	for (size_t i = 0; i < n; i++) {
		printf("%*s", 3 * (int)i, "");
		for (size_t j = i; j < n; j++) {
			size_t idx = triangle_idx(i, j);

			printf("%2d ", triangle_arr[idx]);
		}
		printf("\n");
	}

	free(triangle_arr);

	return 0;
}
