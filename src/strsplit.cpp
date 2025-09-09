#include <assert.h>
#include <stdlib.h>

#include "strfuncs.h"
#include "strstr.h"

#include "strsplit.h"

char *my_strsplit_r(char *str, const char *delim, char **saveptr) {
	assert(delim);
	assert(saveptr);

	if (str != NULL)
		*saveptr = str;

	if (*saveptr == NULL)
		return NULL;

	char *substr_ptr = NULL;
	char *old_saveptr = NULL;
	size_t delim_len = my_strlen(delim);

	do {
		old_saveptr = *saveptr;

		substr_ptr = my_strstr_trivial(*saveptr, delim);

		if (substr_ptr == NULL) {
			*saveptr = NULL;
			break;
		}

		*substr_ptr = '\0';
		*saveptr = substr_ptr + delim_len;
	} while (substr_ptr == old_saveptr);

	if (*old_saveptr == '\0')
		return NULL;

	return old_saveptr;
}
