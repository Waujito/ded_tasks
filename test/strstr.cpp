#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "test_config.h"

#include "strfuncs.h"
#include "strstr.h"

static double millis_diff (struct timespec end_time, struct timespec start_time) {
	double sec_diff = (double)(end_time.tv_sec - start_time.tv_sec);
	double nsec_diff = (double)(end_time.tv_nsec - start_time.tv_nsec);
	double millisec_diff = sec_diff * 1'000 + nsec_diff / 1'000'000;

	return millisec_diff;
}

static int random_gen_and_test(
	const char alphabet[], size_t needle_len, size_t haystack_len, 
	strstr_function my_strstr_fn) {

	char *needle = (char *)calloc(needle_len, 1);
	if (needle == NULL) {
		return -1;
	}

	char *haystack = (char *)calloc(haystack_len, 1);
	if (haystack == NULL) {
		free(needle);
		return -1;
	}

	int alph_len = (int) my_strlen(alphabet);
	for (size_t i = 0; i < needle_len - 1; i++) {
		long rnd = random();
		long rix = rnd % alph_len;
		needle[i] = alphabet[rix];
	}
	needle[needle_len - 1] = 0;

	for (size_t i = 0; i < haystack_len - 1; i++) {
		long rnd = random();
		long rix = rnd % alph_len;
		haystack[i] = alphabet[rix];
	}
	haystack[haystack_len - 1] = 0;

	char *stdlib_found = strstr(haystack, needle);

	struct timespec start_time = {0};
	if (!timespec_get(&start_time, TIME_UTC)) return -1;
	char *my_found = my_strstr_fn(haystack, needle);
	struct timespec end_time = {0};
	if (!timespec_get(&end_time, TIME_UTC)) return -1;

#ifdef TEST_DEBUG
	double millisec_diff = millis_diff(end_time, start_time);
	printf_debug_log("my_strstr_fn worked in %g ms\n", millisec_diff);
#endif /* TEST_DEBUG */


	free(haystack);
	free(needle);

	if (my_found != stdlib_found) {
		return -1;
	}

	
	return stdlib_found != NULL;
}

static int test_strstr(strstr_function fn) {
	int failed = 0;
	int succ_null = 0;
	int succ_cmp = 0;

	struct timespec start_time = {0};
	if (!timespec_get(&start_time, TIME_UTC)) return -1;


	for (int i = 0; i < 100; i++) {
		switch (random_gen_and_test("abcde", 10, 1'200'000, fn)) {	
			case 0:
				succ_null++;
				break;
			case 1:
				succ_cmp++;
				break;
			case -1:
			default:
				failed++;
				break;
		}
	}

	struct timespec end_time = {0};
	if (!timespec_get(&end_time, TIME_UTC)) return -1;

	double millisec_diff = millis_diff(end_time, start_time);

	printf("Failed: %d; Success null: %d; Success ptr: %d\n", failed, succ_null, succ_cmp);
	printf("Tests completed in %g ms\n", millisec_diff);

	ASSERT_EQ(failed, 0);

	return 0;
}

TEST(StrStr, StrStr_hash)
{
	test_strstr(my_strstr_hash);
}

TEST(StrStr, StrStr_trivial)
{
	test_strstr(my_strstr_trivial);
}

TEST(StrStr, StrStr_zfunction)
{
	test_strstr(my_strstr_zfunction);
}

TEST(StrStr, StrStr_boyer_moore)
{
	test_strstr(my_strstr_boyer_moore);
}
