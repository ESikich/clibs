/*
 * cl_libc.c
 * Purpose: Implement small libc-style byte and string primitives for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_libc.h"

#include <stdint.h>

void *cl_memset(void *dst, int value, size_t n)
{
    unsigned char *out = (unsigned char *)dst;
    unsigned char byte = (unsigned char)value;
    size_t i;

    if (!dst && n != 0u) {
        return NULL;
    }

    for (i = 0u; i < n; ++i) {
        out[i] = byte;
    }

    return dst;
}

void *cl_memmove(void *dst, const void *src, size_t n)
{
    unsigned char *out = (unsigned char *)dst;
    const unsigned char *in = (const unsigned char *)src;
    uintptr_t out_addr;
    uintptr_t in_addr;
    size_t i;

    if (n == 0u || dst == src) {
        return dst;
    }

    if (!dst || !src) {
        return NULL;
    }

    out_addr = (uintptr_t)out;
    in_addr = (uintptr_t)in;
    if (out_addr > in_addr) {
        for (i = n; i != 0u; --i) {
            out[i - 1u] = in[i - 1u];
        }
    } else {
        for (i = 0u; i < n; ++i) {
            out[i] = in[i];
        }
    }

    return dst;
}

void *cl_memcpy(void *dst, const void *src, size_t n)
{
    return cl_memmove(dst, src, n);
}

int cl_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *left = (const unsigned char *)a;
    const unsigned char *right = (const unsigned char *)b;
    size_t i;

    if (n == 0u || a == b) {
        return 0;
    }

    if (!a) {
        return -1;
    }

    if (!b) {
        return 1;
    }

    for (i = 0u; i < n; ++i) {
        if (left[i] != right[i]) {
            return left[i] < right[i] ? -1 : 1;
        }
    }

    return 0;
}

size_t cl_strlen(const char *s)
{
    size_t n = 0u;

    if (!s) {
        return 0u;
    }

    while (s[n] != '\0') {
        ++n;
    }

    return n;
}

int cl_strcmp(const char *a, const char *b)
{
    const unsigned char *left = (const unsigned char *)a;
    const unsigned char *right = (const unsigned char *)b;
    size_t i = 0u;

    if (a == b) {
        return 0;
    }

    if (!a) {
        return -1;
    }

    if (!b) {
        return 1;
    }

    while (left[i] == right[i]) {
        if (left[i] == '\0') {
            return 0;
        }
        ++i;
    }

    return left[i] < right[i] ? -1 : 1;
}

char *cl_strchr(const char *s, int c)
{
    unsigned char target = (unsigned char)c;
    size_t i = 0u;

    if (!s) {
        return NULL;
    }

    for (;;) {
        if ((unsigned char)s[i] == target) {
            return (char *)(s + i);
        }

        if (s[i] == '\0') {
            return NULL;
        }

        ++i;
    }
}
