/*
 * cl_utf8.h
 * Purpose: UTF-8 validation, iteration, and encoding helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_UTF8_H
#define CL_UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CL_UTF8_MAX_BYTES 4u
#define CL_UTF8_MAX_CODEPOINT UINT32_C(0x10FFFF)
#define CL_UTF8_REPLACEMENT_CHAR UINT32_C(0xFFFD)

typedef enum cl_utf8_status {
    CL_UTF8_OK = 0,
    CL_UTF8_END,
    CL_UTF8_INVALID,
    CL_UTF8_TRUNCATED,
    CL_UTF8_OVERLONG,
    CL_UTF8_SURROGATE,
    CL_UTF8_OUT_OF_RANGE
} cl_utf8_status;

typedef struct cl_utf8_iter {
    const char *data;
    size_t size;
    size_t offset;
} cl_utf8_iter;

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

#ifdef __cplusplus
}
#endif

#endif
