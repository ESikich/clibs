/*
 * cl_libc.h
 * Purpose: Small libc-style byte and string primitives for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_LIBC_H
#define CL_LIBC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *cl_memset(void *dst, int value, size_t n);
void *cl_memcpy(void *dst, const void *src, size_t n);
void *cl_memmove(void *dst, const void *src, size_t n);
int cl_memcmp(const void *a, const void *b, size_t n);

size_t cl_strlen(const char *s);
int cl_strcmp(const char *a, const char *b);
char *cl_strchr(const char *s, int c);

#ifdef __cplusplus
}
#endif

#endif
