#ifndef STRFUNCS_H
#define STRFUNCS_H

#include <stdlib.h>
#include <stdio.h>


size_t my_strlen(const char *s);

ssize_t my_puts(const char *s);

char *my_strchr(char *s, int c);

char *my_strcpy(char *const dst, const char *src);

char *my_strncpy(char *dest, const char *src, size_t n);

char *my_strcat(char *dest, const char *src);

char *my_strncat(char *dest, const char *src, size_t n);

char *my_strdup(const char *s);

ssize_t my_getline(char **lineptr, size_t *n, FILE *stream);

char *my_strtok_r(char *str, const char *delim, char **saveptr);

#endif /* STRFUNCS_H */
