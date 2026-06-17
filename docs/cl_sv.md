<!--
cl_sv.md
Purpose: String-view library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_sv

`cl_sv` provides non-owning string slices for byte-oriented text. A view is a
pointer plus length; it does not allocate, copy, or require null termination.

## Interface

```c
typedef struct cl_sv {
    const char *data;
    size_t size;
} cl_sv;

typedef enum cl_sv_parse_status {
    CL_SV_PARSE_OK = 0,
    CL_SV_PARSE_EMPTY,
    CL_SV_PARSE_INVALID,
    CL_SV_PARSE_OVERFLOW
} cl_sv_parse_status;

#define CL_SV_LIT(s) ((cl_sv){ (s), sizeof(s) - 1u })

cl_sv cl_sv_from_parts(const char *data, size_t size);
cl_sv cl_sv_from_cstr(const char *s);
bool cl_sv_is_empty(cl_sv s);
bool cl_sv_is_valid(cl_sv s);

cl_sv cl_sv_trim_left(cl_sv s);
cl_sv cl_sv_trim_right(cl_sv s);
cl_sv cl_sv_trim(cl_sv s);

bool cl_sv_eq(cl_sv a, cl_sv b);
int cl_sv_cmp(cl_sv a, cl_sv b);
bool cl_sv_starts_with(cl_sv s, cl_sv prefix);
bool cl_sv_ends_with(cl_sv s, cl_sv suffix);

bool cl_sv_split_once(cl_sv s, char delimiter, cl_sv *before, cl_sv *after);
bool cl_sv_next_split(cl_sv *rest, char delimiter, cl_sv *part);

cl_sv_parse_status cl_sv_parse_u64(cl_sv s, uint64_t *out);
cl_sv_parse_status cl_sv_parse_i64(cl_sv s, int64_t *out);
```

## Contracts

Views borrow caller-owned memory. Callers must keep the pointed-to bytes alive
while a view is used.

`cl_sv_from_parts(NULL, nonzero)` returns an empty view. `cl_sv_is_valid`
reports whether a view has either a non-null pointer or zero length; functions
that receive an invalid manually-constructed view fail safely instead of reading
through the null pointer.

Trimming recognizes ASCII whitespace only: space, tab, newline, carriage return,
vertical tab, and form feed. Comparisons are byte-wise and unsigned-ordering
compatible.

`cl_sv_split_once` finds the first delimiter and returns the two sides without
including the delimiter. `cl_sv_next_split` is useful for iteration and preserves
empty fields between adjacent delimiters.

Integer parsing trims leading and trailing ASCII whitespace, accepts decimal
digits only, reports empty/invalid/overflow distinctly, and writes `out` only on
success. Signed parsing accepts one leading `+` or `-`.

## Portability

The implementation is C99 and does not use locale, heap allocation, `errno`, or
non-POSIX APIs.
