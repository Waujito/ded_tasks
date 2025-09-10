#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "strstr.h"
#include "strfuncs.h"

char *my_strstr_trivial(char *haystack, const char *needle) {
	assert (haystack);
	assert (needle);

	do {
		const char *hay_ctx_ptr = haystack;
		const char *needle_ctx_ptr = needle;

		while (	*hay_ctx_ptr != '\0' && *needle_ctx_ptr != '\0' &&
			*(hay_ctx_ptr) == *(needle_ctx_ptr))
			hay_ctx_ptr++, needle_ctx_ptr++;

		if (*needle_ctx_ptr == '\0') {
			return (char *)haystack;
		}
	} while (*(haystack++) != '\0');

	return NULL;
}

static void z_function(const char *str, int *zbuf, size_t str_len) {
	zbuf[0] = (int) str_len;

	int left_border = 0, right_border = 0;

	for (int i = 1; i < (int) str_len; i++) {
		zbuf[i] = 0;
		if (i < right_border) {
			zbuf[i] = zbuf[i - left_border];
			if (right_border - i < zbuf[i])
				zbuf[i] = right_border - i;
		}

		while (	i + zbuf[i] < (int) str_len &&
			str[zbuf[i]] == str[i + zbuf[i]])
			zbuf[i]++;

		if (i + zbuf[i] > right_border) {
			left_border = i;
			right_border = i + zbuf[i];
		}
	}
}

char *my_strstr_zfunction(char *haystack, const char *needle) {
	assert (haystack);
	assert (needle);

	size_t haystack_len = my_strlen(haystack);
	size_t needle_len = my_strlen(needle);

	size_t kmp_len = needle_len + haystack_len;

	// One for null-terminator
	char *kmp_buf = (char *)calloc(kmp_len + 1, sizeof(char));
	if (!kmp_buf) {
		perror("strstr_zfunction calloc");
		return NULL;
	}

	int *zbuf = (int *)calloc(kmp_len + 1, sizeof(int));
	if (!zbuf) {
		perror("strstr_zfunction calloc");
		return NULL;
	}

	my_strcat(kmp_buf, needle);
	my_strcat(kmp_buf, haystack);

	z_function(kmp_buf, zbuf, kmp_len);

	for (size_t i = 0; i < haystack_len; i++) {
		if ((size_t) zbuf[needle_len + i] >= needle_len) {
			free(kmp_buf);
			free(zbuf);
			return haystack + i;
		}
	}

	free(kmp_buf);
	free(zbuf);

	return NULL;
}

typedef int64_t hash_t;
static const hash_t HASH_MOD = 1'000'000'007;

struct hash_st {
	hash_t hash;
	hash_t hash_slide;
};

static inline hash_t positive_mod(hash_t a, hash_t b) {
    hash_t result = a % b;
    if (result < 0) {
        result += (b < 0) ? -b : b;
    }
    return result;
}

static const hash_t ALPHABET_STRENGTH = 256;

static inline struct hash_st calc_hash(const char *str, size_t needle_len) {
	assert(str);

	hash_t hash = (hash_t) str[0];
	hash_t hash_slide = 1;

	for (size_t i = 1; i < needle_len; i++) {
		hash = ((hash * ALPHABET_STRENGTH) % HASH_MOD + (hash_t) str[i]) % HASH_MOD;
		hash_slide = (hash_slide * ALPHABET_STRENGTH) % HASH_MOD;
	}

	struct hash_st hs = {
		.hash = hash,
		.hash_slide = hash_slide,
	};

	return hs;
}

static inline struct hash_st slide_hash(
	const char *str, size_t i, size_t needle_len, struct hash_st hs) {

	assert(str);

	hash_t hash_slide = hs.hash_slide;
	hash_slide = (hash_slide * str[i - needle_len]) % HASH_MOD;

	hash_t nhash = hs.hash;
	nhash = positive_mod(nhash - hash_slide, HASH_MOD);
	nhash = (((nhash * ALPHABET_STRENGTH) % HASH_MOD) + str[i]) % HASH_MOD;
	hs.hash = nhash;

	return hs;
}

char *my_strstr_hash(char *haystack, const char *needle) {
	assert (haystack);
	assert (needle);

	size_t haystack_len = my_strlen(haystack);
	size_t needle_len = my_strlen(needle);

	struct hash_st hs_target = calc_hash(needle, needle_len);
	struct hash_st hs = calc_hash(haystack, needle_len);

	if (hs.hash == hs_target.hash && !strncmp(haystack, needle, needle_len)) {
		return (char *)haystack;
	}

	for (size_t i = needle_len; i < haystack_len; i++) {
		hs = slide_hash(haystack, i, needle_len, hs);

		if (hs.hash == hs_target.hash) {
			char *target_ptr = haystack + i - needle_len + 1;

			if (strncmp(target_ptr, needle, needle_len)) {
				printf("Collision\n");
				continue;
			}

			return (char *)target_ptr;
		}
	}

	return NULL;
}

