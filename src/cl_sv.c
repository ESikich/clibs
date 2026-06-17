/*
 * cl_sv.c
 * Purpose: Implement non-owning string-view helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_sv.h"

#include <stdint.h>

static bool cl_sv_is_space(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\n' ||
           c == '\r' || c == '\v' || c == '\f';
}

static cl_sv cl_sv_empty(void)
{
    cl_sv out;

    out.data = NULL;
    out.size = 0u;
    return out;
}

static cl_sv_parse_status cl_sv_parse_u64_limit(
    cl_sv s,
    uint64_t limit,
    uint64_t *out)
{
    uint64_t value = 0u;
    size_t i;

    if (!cl_sv_is_valid(s)) {
        return CL_SV_PARSE_INVALID;
    }

    s = cl_sv_trim(s);
    if (s.size == 0u) {
        return CL_SV_PARSE_EMPTY;
    }

    for (i = 0u; i < s.size; ++i) {
        unsigned char c = (unsigned char)s.data[i];
        uint64_t digit;

        if (c < (unsigned char)'0' || c > (unsigned char)'9') {
            return CL_SV_PARSE_INVALID;
        }

        digit = (uint64_t)(c - (unsigned char)'0');
        if (value > (limit - digit) / 10u) {
            return CL_SV_PARSE_OVERFLOW;
        }
        value = (value * 10u) + digit;
    }

    if (out) {
        *out = value;
    }
    return CL_SV_PARSE_OK;
}

cl_sv cl_sv_from_parts(const char *data, size_t size)
{
    cl_sv out;

    if (!data && size != 0u) {
        return cl_sv_empty();
    }

    out.data = data;
    out.size = size;
    return out;
}

cl_sv cl_sv_from_cstr(const char *s)
{
    size_t size = 0u;

    if (!s) {
        return cl_sv_empty();
    }

    while (s[size] != '\0') {
        ++size;
    }

    return cl_sv_from_parts(s, size);
}

bool cl_sv_is_empty(cl_sv s)
{
    return s.size == 0u;
}

bool cl_sv_is_valid(cl_sv s)
{
    return s.data != NULL || s.size == 0u;
}

cl_sv cl_sv_trim_left(cl_sv s)
{
    size_t offset = 0u;

    if (!cl_sv_is_valid(s)) {
        return cl_sv_empty();
    }

    if (s.size == 0u) {
        return cl_sv_empty();
    }

    while (offset < s.size && cl_sv_is_space((unsigned char)s.data[offset])) {
        ++offset;
    }

    return cl_sv_from_parts(s.data + offset, s.size - offset);
}

cl_sv cl_sv_trim_right(cl_sv s)
{
    size_t size;

    if (!cl_sv_is_valid(s)) {
        return cl_sv_empty();
    }

    size = s.size;
    while (size > 0u && cl_sv_is_space((unsigned char)s.data[size - 1u])) {
        --size;
    }

    return cl_sv_from_parts(s.data, size);
}

cl_sv cl_sv_trim(cl_sv s)
{
    return cl_sv_trim_right(cl_sv_trim_left(s));
}

bool cl_sv_eq(cl_sv a, cl_sv b)
{
    size_t i;

    if (!cl_sv_is_valid(a) || !cl_sv_is_valid(b)) {
        return false;
    }

    if (a.size != b.size) {
        return false;
    }

    if (a.size == 0u) {
        return true;
    }

    for (i = 0u; i < a.size; ++i) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }

    return true;
}

int cl_sv_cmp(cl_sv a, cl_sv b)
{
    size_t min_size;
    size_t i;

    if (!cl_sv_is_valid(a)) {
        a = cl_sv_empty();
    }
    if (!cl_sv_is_valid(b)) {
        b = cl_sv_empty();
    }

    min_size = a.size < b.size ? a.size : b.size;
    for (i = 0u; i < min_size; ++i) {
        unsigned char left = (unsigned char)a.data[i];
        unsigned char right = (unsigned char)b.data[i];

        if (left != right) {
            return left < right ? -1 : 1;
        }
    }

    if (a.size == b.size) {
        return 0;
    }
    return a.size < b.size ? -1 : 1;
}

bool cl_sv_starts_with(cl_sv s, cl_sv prefix)
{
    size_t i;

    if (!cl_sv_is_valid(s) || !cl_sv_is_valid(prefix) || prefix.size > s.size) {
        return false;
    }

    for (i = 0u; i < prefix.size; ++i) {
        if (s.data[i] != prefix.data[i]) {
            return false;
        }
    }
    return true;
}

bool cl_sv_ends_with(cl_sv s, cl_sv suffix)
{
    size_t offset;
    size_t i;

    if (!cl_sv_is_valid(s) || !cl_sv_is_valid(suffix) || suffix.size > s.size) {
        return false;
    }

    offset = s.size - suffix.size;
    for (i = 0u; i < suffix.size; ++i) {
        if (s.data[offset + i] != suffix.data[i]) {
            return false;
        }
    }
    return true;
}

bool cl_sv_split_once(cl_sv s, char delimiter, cl_sv *before, cl_sv *after)
{
    size_t i;

    if (!cl_sv_is_valid(s)) {
        return false;
    }

    for (i = 0u; i < s.size; ++i) {
        if (s.data[i] == delimiter) {
            if (before) {
                *before = cl_sv_from_parts(s.data, i);
            }
            if (after) {
                *after = cl_sv_from_parts(s.data + i + 1u, s.size - i - 1u);
            }
            return true;
        }
    }

    return false;
}

bool cl_sv_next_split(cl_sv *rest, char delimiter, cl_sv *part)
{
    cl_sv before;
    cl_sv after;

    if (!rest || !cl_sv_is_valid(*rest) ||
        (rest->data == NULL && rest->size == 0u)) {
        return false;
    }

    if (cl_sv_split_once(*rest, delimiter, &before, &after)) {
        if (part) {
            *part = before;
        }
        *rest = after;
        return true;
    }

    if (part) {
        *part = *rest;
    }
    *rest = cl_sv_empty();
    return true;
}

cl_sv_parse_status cl_sv_parse_u64(cl_sv s, uint64_t *out)
{
    return cl_sv_parse_u64_limit(s, UINT64_MAX, out);
}

cl_sv_parse_status cl_sv_parse_i64(cl_sv s, int64_t *out)
{
    bool negative = false;
    uint64_t magnitude;
    uint64_t limit = (uint64_t)INT64_MAX;
    cl_sv_parse_status status;

    if (!cl_sv_is_valid(s)) {
        return CL_SV_PARSE_INVALID;
    }

    s = cl_sv_trim(s);
    if (s.size == 0u) {
        return CL_SV_PARSE_EMPTY;
    }

    if (s.data[0] == '-' || s.data[0] == '+') {
        negative = s.data[0] == '-';
        s = cl_sv_from_parts(s.data + 1u, s.size - 1u);
        if (s.size == 0u) {
            return CL_SV_PARSE_INVALID;
        }
    }

    if (negative) {
        limit = ((uint64_t)INT64_MAX) + 1u;
    }

    status = cl_sv_parse_u64_limit(s, limit, &magnitude);
    if (status != CL_SV_PARSE_OK) {
        return status;
    }

    if (out) {
        if (negative && magnitude == (((uint64_t)INT64_MAX) + 1u)) {
            *out = INT64_MIN;
        } else if (negative) {
            *out = -(int64_t)magnitude;
        } else {
            *out = (int64_t)magnitude;
        }
    }

    return CL_SV_PARSE_OK;
}
