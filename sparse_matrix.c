#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

union coo_primitive {
	void *ptr;
	int n;
};

struct coo_entry {
	size_t i, j;
	union coo_primitive p;
};

struct coo_vector {
	struct coo_entry *entries;
	size_t coo_capacity;
	size_t coo_length;
};

#define STARTING_COO_CAPACITY (128)
int init_coo_vector(struct coo_vector *vector) {
	assert(vector);

	vector->coo_capacity	= 0;
	vector->coo_length	= 0;
	vector->entries		= NULL;

	return 0;
}

int coo_vector_push(struct coo_vector *vector, struct coo_entry entry) {
	assert(vector);

	if (vector->coo_length >= vector->coo_capacity) {
		size_t new_capacity = vector->coo_capacity * 2;
		if (new_capacity < STARTING_COO_CAPACITY) {
			new_capacity = STARTING_COO_CAPACITY;
		}

		struct coo_entry *new_entries = (struct coo_entry *)realloc(vector->entries, new_capacity * sizeof (struct coo_entry));
		if (!new_entries) {
			return -1;
		}

		vector->entries = new_entries;
		vector->coo_capacity = new_capacity;
	}
	
	vector->entries[vector->coo_length++] = entry;

	return 0;
}

void destroy_coo_vector(struct coo_vector *vector) {
	free(vector->entries);
}

int find_matrix_element(struct coo_vector *vector, struct coo_entry **entry_to,
		       size_t i, size_t j) {
	assert(vector);
	assert(entry_to);

	for (size_t v_idx = 0; v_idx < vector->coo_length; v_idx++) {
		struct coo_entry *entry = vector->entries + v_idx;
		if (entry->i == i && entry->j == j) {
			*entry_to = entry;

			return 1;
		}
	}

	*entry_to = NULL;

	return 0;
}

int get_matrix_element(struct coo_vector *vector, struct coo_entry *entry_to,
		       size_t i, size_t j) {
	assert(vector);

	for (size_t v_idx = 0; v_idx < vector->coo_length; v_idx++) {
		struct coo_entry *entry = vector->entries + v_idx;
		if (entry->i == i && entry->j == j) {
			if (entry_to)
				*entry_to = *entry;

			return 1;
		}
	}

	if (entry_to) {
		entry_to->i = i;
		entry_to->j = j;
		entry_to->p = {0};
	}

	return 0;
}

int set_matrix_element(struct coo_vector *vector, struct coo_entry *entry_to,
		       size_t i, size_t j) {
	assert(vector);

	for (size_t v_idx = 0; v_idx < vector->coo_length; v_idx++) {
		struct coo_entry *entry = vector->entries + v_idx;
		if (entry->i == i && entry->j == j) {
			if (entry_to)
				*entry_to = *entry;

			return 1;
		}
	}

	return 0;
}

int main() {

}
