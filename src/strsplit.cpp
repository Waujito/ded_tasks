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

	char *delim_substr_ptr = NULL;
	char *token_beginning  = NULL;
	size_t delim_len = my_strlen(delim);

	do {
		token_beginning = *saveptr;

		delim_substr_ptr = my_strstr_trivial(*saveptr, delim);

		if (delim_substr_ptr == NULL) {
			*saveptr = NULL;
			break;
		}

		*delim_substr_ptr = '\0';
		*saveptr = delim_substr_ptr + delim_len;

	} while (delim_substr_ptr == token_beginning);

	if (*token_beginning == '\0')
		return NULL;

	return token_beginning;
}
