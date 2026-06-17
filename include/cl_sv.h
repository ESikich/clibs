/*
 * cl_sv.h
 * Purpose: Non-owning string-view helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_SV_H
#define CL_SV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
