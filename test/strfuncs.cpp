#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "test_config.h"

#include "strfuncs.h"

TEST(StrFuncs, StrLen)
{
	size_t len = my_strlen("abacaba\0");
	ASSERT_EQ((int) len, 7);
}

static void test_strtok_r(const char *strtok_test_string, const char *strtok_test_delim) {
	char *my_saveptr = NULL, *stdlib_saveptr = NULL;

	char *my_test_string = (char *)calloc(strlen(strtok_test_string) + 1, 1);
	if (my_test_string == NULL) {
		ASSERT_EQ(0, 1);
	}

	char *stdlib_test_string = (char *)calloc(strlen(strtok_test_string) + 1, 1);
	if (stdlib_test_string == NULL) {
		free(my_test_string);
		ASSERT_EQ(0, 1);
	}

	strcpy(my_test_string, strtok_test_string);
	strcpy(stdlib_test_string, strtok_test_string);

	char *my_test_ptr	= my_test_string;
	char *stdlib_test_ptr	= stdlib_test_string;

	char *my_stt = NULL, *stdlib_stt = NULL;

        for (;;) {
		my_stt = my_strtok_r(my_test_ptr, strtok_test_delim, &my_saveptr);
		stdlib_stt = strtok_r(stdlib_test_ptr, strtok_test_delim, 
					&stdlib_saveptr);

		if (my_stt == NULL) {
			ASSERT_EQ((int)(stdlib_stt == NULL), 1);
			break;
		}

		my_test_ptr	= NULL;
		stdlib_test_ptr	= NULL;

		printf("<%s> <%s>\n", my_stt, stdlib_stt);
		ASSERT_EQ((int)(stdlib_stt != NULL), 1);
		ASSERT_EQ((int)(
			(my_stt - my_test_string) == (stdlib_stt - stdlib_test_string)
		), 1);
        }

	free(my_test_string);
	free(stdlib_test_string);

}

TEST(StrFuncs, Strtok_r)
{	
	test_strtok_r(
		"abacabafdabajlkewjrabajlkjabaaaaaaaaabaabaddaaabadddabaaaaba", "aba");
}

