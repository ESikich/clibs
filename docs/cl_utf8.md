<!--
docs/cl_utf8.md
Purpose: UTF-8 helper library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_utf8

`cl_utf8` provides allocation-free UTF-8 validation, decoding, encoding, and
iteration over caller-owned byte spans.

## Interface

```c
bool cl_utf8_is_valid_codepoint(uint32_t codepoint);
cl_utf8_status cl_utf8_decode(
    const char *data,
    size_t size,
    uint32_t *codepoint,
    size_t *consumed);
cl_utf8_status cl_utf8_encode(
    uint32_t codepoint,
    char *out,
    size_t out_size,
    size_t *written);
bool cl_utf8_validate(const char *data, size_t size);
size_t cl_utf8_count_codepoints(const char *data, size_t size);

void cl_utf8_iter_init(cl_utf8_iter *it, const char *data, size_t size);
cl_utf8_status cl_utf8_iter_next(
    cl_utf8_iter *it,
    uint32_t *codepoint,
    size_t *byte_offset);
```

## Contracts

Byte spans permit `data == NULL` only when `size == 0`. Validation rejects bad
continuation bytes, truncated sequences, overlong encodings, UTF-16 surrogate
code points, and code points above `U+10FFFF`.

`cl_utf8_decode` reads one scalar value from the front of a byte span. On
success it writes the decoded code point and byte count when those output
pointers are supplied.

`cl_utf8_encode` writes the shortest UTF-8 encoding for one valid scalar value.
It returns `CL_UTF8_TRUNCATED` when `out_size` is too small and leaves `written`
as zero on failure.

`cl_utf8_iter_next` returns `CL_UTF8_END` after the last byte. On malformed input
it reports the failing byte offset and does not advance the iterator, allowing
callers to diagnose or recover at the exact byte that failed.

`cl_utf8_count_codepoints` returns the number of scalar values for valid input
and zero for malformed input. Empty input is valid and also counts as zero, so
use `cl_utf8_validate` first when that distinction matters.

## Portability

The implementation is byte-wise C99 and does not depend on locale, wide
characters, compiler intrinsics, or non-POSIX APIs.
