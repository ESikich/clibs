<!--
docs/cl_libc.md
Purpose: libc-mini library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_libc

`cl_libc` provides small prefixed byte and string primitives for environments
where the project wants predictable local implementations.

## Interface

```c
void *cl_memset(void *dst, int value, size_t n);
void *cl_memcpy(void *dst, const void *src, size_t n);
void *cl_memmove(void *dst, const void *src, size_t n);
int cl_memcmp(const void *a, const void *b, size_t n);

size_t cl_strlen(const char *s);
int cl_strcmp(const char *a, const char *b);
char *cl_strchr(const char *s, int c);
```

## Contracts

Memory functions permit null pointers only when `n == 0`. For non-zero byte
counts, `cl_memset`, `cl_memcpy`, and `cl_memmove` return `NULL` if a required
pointer is missing. `cl_memcmp` orders null below non-null for non-zero byte
counts.

`cl_memcpy` is overlap-safe and currently shares `cl_memmove` behavior. This is
stricter than standard `memcpy`, but avoids accidental undefined behavior in
callers that use the project-local primitive.

String functions expect null-terminated byte strings. `cl_strlen(NULL)` returns
zero, `cl_strcmp` orders null below non-null, and `cl_strchr(NULL, c)` returns
`NULL`.

## Portability

The implementation is byte-wise C99 and does not depend on locale, compiler
intrinsics, or non-POSIX APIs.
