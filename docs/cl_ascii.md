<!--
docs/cl_ascii.md
Purpose: ASCII helper library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_ascii

`cl_ascii` provides locale-free classification, case conversion, and digit value
helpers for individual ASCII bytes represented as `int` values.

## Interface

```c
bool cl_ascii_is_ascii(int c);
bool cl_ascii_is_control(int c);
bool cl_ascii_is_space(int c);
bool cl_ascii_is_blank(int c);
bool cl_ascii_is_digit(int c);
bool cl_ascii_is_xdigit(int c);
bool cl_ascii_is_lower(int c);
bool cl_ascii_is_upper(int c);
bool cl_ascii_is_alpha(int c);
bool cl_ascii_is_alnum(int c);
bool cl_ascii_is_print(int c);
bool cl_ascii_is_graph(int c);
bool cl_ascii_is_punct(int c);

int cl_ascii_to_lower(int c);
int cl_ascii_to_upper(int c);
int cl_ascii_digit_value(int c);
int cl_ascii_hex_value(int c);
bool cl_ascii_equal_ignore_case(int a, int b);
```

## Contracts

All helpers are pure byte-range checks and do not inspect or depend on the
current locale. Inputs outside ASCII are classified as false except for
conversion helpers, which return unchanged input, and value helpers, which
return `-1`.

`cl_ascii_is_space` matches the ASCII whitespace set: space, form feed, newline,
carriage return, horizontal tab, and vertical tab. `cl_ascii_is_blank` matches
only space and horizontal tab.

Case conversion affects only `A` through `Z` and `a` through `z`.
`cl_ascii_equal_ignore_case` applies that same ASCII-only conversion before
comparison.

## Portability

The implementation uses only C99 integer comparisons and avoids `<ctype.h>` so
behavior is stable for negative values, bytes above `0x7f`, and every POSIX
locale.
