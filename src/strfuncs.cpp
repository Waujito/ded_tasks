#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "strfuncs.h"


#define STDOUT_FD (1)

size_t my_strlen(const char *s) {
	assert (s);

	size_t len = 0;
	while (*(s++) != '\0')
		++len;

	return len;
}

ssize_t my_puts(const char *s) {
	assert (s);

	size_t len = my_strlen(s);

	ssize_t bwritten = write(STDOUT_FD, s, len);
	if (bwritten < 0) {
		return bwritten;
	}

	if (bwritten != (ssize_t) len) {
		return -1;
	}

	ssize_t nlwritten = write(STDOUT_FD, "\n", 1);
	if (nlwritten < 0) {
		return nlwritten;
	}

	bwritten += nlwritten;

	return bwritten;
}

char *my_strchr(char *s, int c) {
	assert (s);

	do {
		if (*s == c) {
			return s;
		}
	} while (*(s++) != '\0');

	return NULL;
}

char *my_strcpy(char *const dst, const char *src) {
	assert (dst);
	assert (src);

	size_t i = 0;

	for (i = 0;; i++) {
		dst[i] = src[i];
		if (src[i] == '\0')
			break;
	}

	return dst;
}

char *my_strncpy(char *dest, const char *src, size_t n) {
	assert (dest);
	assert (src);

	for (size_t i = 0; i < n; i++) {
		dest[i] = src[i];
		if (src[i] == '\0') 
			break;
	}

	return dest;
}

char *my_strcat(char *dest, const char *src) {
	assert (dest);
	assert (src);

	size_t dest_len = my_strlen(dest);
	size_t i = 0;

	do {
		dest[dest_len + i] = src[i];
	} while (src[i++] != '\0');

	return dest;
}

char *my_strncat(char *dest, const char *src, size_t n) {
	assert (dest);
	assert (src);

	size_t dest_len = my_strlen(dest);
	size_t i = 0;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[dest_len + i] = src[i];
	}
	dest[dest_len + i] = '\0';

	return dest;
}

char *my_strdup(const char *s) {
	assert (s);

	size_t strlen = my_strlen(s);
	char *dup = (char *)malloc(strlen + 1);
	if (dup == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for (size_t i = 0; i <= strlen; i++) {
		dup[i] = s[i];
	}

	return dup;
}

#include <stdio.h>

#define STARTING_BUFSIZE 120
ssize_t my_getline(char **lineptr, size_t *n, FILE *stream) {
	if (!lineptr || !n || !stream) {
		errno = EINVAL;
		return -1;
	}

	int ch = 0;
	size_t i = 0;
	while ((ch = fgetc(stream)) != EOF) {
		// Allocate +1 char for null-terminator
		if (i + 1 >= *n) {
			size_t new_sz = *n * 2;
			if (new_sz < STARTING_BUFSIZE)
				new_sz = STARTING_BUFSIZE;

			char *new_lm = (char *)realloc(*lineptr, 
				  new_sz);
			if (!new_lm) {
				errno = ENOMEM;
				return -1;
			}

			*lineptr = new_lm;
			*n = new_sz;
		}

		(*lineptr)[i++] = (char) ch;

		if (ch == '\n') {
			break;
		}
	}

	// Allowed because of guarranteed len + 1 allocation
	(*lineptr)[i] = '\0';

	if (ch == EOF) {
		if (errno) {
			errno = EINVAL;
		}

		return -1;
	}

	return (ssize_t) i;
}

char *my_strtok_r(char *str, const char *delim, char **saveptr) {
	assert(delim);
	assert(saveptr);

	if (str != NULL)
		*saveptr = str;

	if (*saveptr == NULL)
		return NULL;

	char *token_beginning = *saveptr;
	while (*token_beginning != '\0') {
		const char *delim_found = strchr(delim, *token_beginning);
		if (delim_found) {
			*token_beginning = '\0';

			if (*saveptr == token_beginning) {
				(*saveptr)++;
				token_beginning++;
				continue;
			} else {
				token_beginning++;
				break;
			}

		} else {
			token_beginning++;
		}
	}

	char *old_saveptr = *saveptr;

	if (*token_beginning == '\0') {
		token_beginning = NULL;
	}

	*saveptr = token_beginning;

	if (*old_saveptr == '\0')
		return NULL;

	return old_saveptr;
}
