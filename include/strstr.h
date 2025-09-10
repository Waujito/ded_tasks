#ifndef STRSTR_H
#define STRSTR_H

#include "stdlib.h"

typedef char *(*strstr_function)(char *haystack, const char *needle);

char *my_strstr_trivial(char *haystack, const char *needle);

char *my_strstr_zfunction(char *haystack, const char *needle);

char *my_strstr_hash(char *haystack, const char *needle);

char *my_strstr_boyer_moore(char *haystack, const char *needle);

#endif /* STRSTR_H */

