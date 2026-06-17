/*
 * cl_utf8.c
 * Purpose: Implement UTF-8 validation, iteration, and encoding helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_utf8.h"

static bool cl_utf8_is_cont(unsigned char c)
{
    return c >= (unsigned char)0x80u && c <= (unsigned char)0xBFu;
}

bool cl_utf8_is_valid_codepoint(uint32_t codepoint)
{
    return codepoint <= CL_UTF8_MAX_CODEPOINT &&
           !(codepoint >= UINT32_C(0xD800) && codepoint <= UINT32_C(0xDFFF));
}

cl_utf8_status cl_utf8_decode(
    const char *data,
    size_t size,
    uint32_t *codepoint,
    size_t *consumed)
{
    const unsigned char *bytes = (const unsigned char *)data;
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
    uint32_t value;

    if (consumed) {
        *consumed = 0u;
    }
    if (codepoint) {
        *codepoint = 0u;
    }

    if (size == 0u) {
        return CL_UTF8_END;
    }
    if (!data) {
        return CL_UTF8_INVALID;
    }

    b0 = bytes[0];
    if (b0 <= (unsigned char)0x7Fu) {
        if (codepoint) {
            *codepoint = (uint32_t)b0;
        }
        if (consumed) {
            *consumed = 1u;
        }
        return CL_UTF8_OK;
    }

    if (b0 >= (unsigned char)0xC2u && b0 <= (unsigned char)0xDFu) {
        if (size < 2u) {
            return CL_UTF8_TRUNCATED;
        }
        b1 = bytes[1];
        if (!cl_utf8_is_cont(b1)) {
            return CL_UTF8_INVALID;
        }
        value = (((uint32_t)b0 & UINT32_C(0x1F)) << 6u) |
                ((uint32_t)b1 & UINT32_C(0x3F));
        if (codepoint) {
            *codepoint = value;
        }
        if (consumed) {
            *consumed = 2u;
        }
        return CL_UTF8_OK;
    }

    if (b0 >= (unsigned char)0xE0u && b0 <= (unsigned char)0xEFu) {
        if (size < 3u) {
            return CL_UTF8_TRUNCATED;
        }
        b1 = bytes[1];
        b2 = bytes[2];
        if (!cl_utf8_is_cont(b1) || !cl_utf8_is_cont(b2)) {
            return CL_UTF8_INVALID;
        }
        if (b0 == (unsigned char)0xE0u && b1 < (unsigned char)0xA0u) {
            return CL_UTF8_OVERLONG;
        }
        if (b0 == (unsigned char)0xEDu && b1 > (unsigned char)0x9Fu) {
            return CL_UTF8_SURROGATE;
        }
        value = (((uint32_t)b0 & UINT32_C(0x0F)) << 12u) |
                (((uint32_t)b1 & UINT32_C(0x3F)) << 6u) |
                ((uint32_t)b2 & UINT32_C(0x3F));
        if (codepoint) {
            *codepoint = value;
        }
        if (consumed) {
            *consumed = 3u;
        }
        return CL_UTF8_OK;
    }

    if (b0 >= (unsigned char)0xF0u && b0 <= (unsigned char)0xF4u) {
        if (size < 4u) {
            return CL_UTF8_TRUNCATED;
        }
        b1 = bytes[1];
        b2 = bytes[2];
        b3 = bytes[3];
        if (!cl_utf8_is_cont(b1) ||
            !cl_utf8_is_cont(b2) ||
            !cl_utf8_is_cont(b3)) {
            return CL_UTF8_INVALID;
        }
        if (b0 == (unsigned char)0xF0u && b1 < (unsigned char)0x90u) {
            return CL_UTF8_OVERLONG;
        }
        if (b0 == (unsigned char)0xF4u && b1 > (unsigned char)0x8Fu) {
            return CL_UTF8_OUT_OF_RANGE;
        }
        value = (((uint32_t)b0 & UINT32_C(0x07)) << 18u) |
                (((uint32_t)b1 & UINT32_C(0x3F)) << 12u) |
                (((uint32_t)b2 & UINT32_C(0x3F)) << 6u) |
                ((uint32_t)b3 & UINT32_C(0x3F));
        if (codepoint) {
            *codepoint = value;
        }
        if (consumed) {
            *consumed = 4u;
        }
        return CL_UTF8_OK;
    }

    if (b0 == (unsigned char)0xC0u || b0 == (unsigned char)0xC1u) {
        return CL_UTF8_OVERLONG;
    }
    if (b0 >= (unsigned char)0xF5u) {
        return CL_UTF8_OUT_OF_RANGE;
    }
    return CL_UTF8_INVALID;
}

cl_utf8_status cl_utf8_encode(
    uint32_t codepoint,
    char *out,
    size_t out_size,
    size_t *written)
{
    unsigned char *bytes = (unsigned char *)out;
    size_t needed;

    if (written) {
        *written = 0u;
    }
    if (codepoint >= UINT32_C(0xD800) && codepoint <= UINT32_C(0xDFFF)) {
        return CL_UTF8_SURROGATE;
    }
    if (codepoint > CL_UTF8_MAX_CODEPOINT) {
        return CL_UTF8_OUT_OF_RANGE;
    }

    if (codepoint <= UINT32_C(0x7F)) {
        needed = 1u;
    } else if (codepoint <= UINT32_C(0x7FF)) {
        needed = 2u;
    } else if (codepoint <= UINT32_C(0xFFFF)) {
        needed = 3u;
    } else {
        needed = 4u;
    }

    if (out_size < needed) {
        return CL_UTF8_TRUNCATED;
    }
    if (!out) {
        return CL_UTF8_INVALID;
    }

    if (needed == 1u) {
        bytes[0] = (unsigned char)codepoint;
    } else if (needed == 2u) {
        bytes[0] = (unsigned char)(UINT32_C(0xC0) | (codepoint >> 6u));
        bytes[1] = (unsigned char)(UINT32_C(0x80) | (codepoint & UINT32_C(0x3F)));
    } else if (needed == 3u) {
        bytes[0] = (unsigned char)(UINT32_C(0xE0) | (codepoint >> 12u));
        bytes[1] = (unsigned char)(UINT32_C(0x80) |
                                   ((codepoint >> 6u) & UINT32_C(0x3F)));
        bytes[2] = (unsigned char)(UINT32_C(0x80) | (codepoint & UINT32_C(0x3F)));
    } else {
        bytes[0] = (unsigned char)(UINT32_C(0xF0) | (codepoint >> 18u));
        bytes[1] = (unsigned char)(UINT32_C(0x80) |
                                   ((codepoint >> 12u) & UINT32_C(0x3F)));
        bytes[2] = (unsigned char)(UINT32_C(0x80) |
                                   ((codepoint >> 6u) & UINT32_C(0x3F)));
        bytes[3] = (unsigned char)(UINT32_C(0x80) | (codepoint & UINT32_C(0x3F)));
    }

    if (written) {
        *written = needed;
    }
    return CL_UTF8_OK;
}

bool cl_utf8_validate(const char *data, size_t size)
{
    cl_utf8_iter it;

    cl_utf8_iter_init(&it, data, size);
    for (;;) {
        cl_utf8_status status = cl_utf8_iter_next(&it, NULL, NULL);

        if (status == CL_UTF8_END) {
            return true;
        }
        if (status != CL_UTF8_OK) {
            return false;
        }
    }
}

size_t cl_utf8_count_codepoints(const char *data, size_t size)
{
    cl_utf8_iter it;
    size_t count = 0u;

    cl_utf8_iter_init(&it, data, size);
    for (;;) {
        cl_utf8_status status = cl_utf8_iter_next(&it, NULL, NULL);

        if (status == CL_UTF8_END) {
            return count;
        }
        if (status != CL_UTF8_OK) {
            return 0u;
        }
        ++count;
    }
}

void cl_utf8_iter_init(cl_utf8_iter *it, const char *data, size_t size)
{
    if (!it) {
        return;
    }

    it->data = data;
    it->size = size;
    it->offset = 0u;
}

cl_utf8_status cl_utf8_iter_next(
    cl_utf8_iter *it,
    uint32_t *codepoint,
    size_t *byte_offset)
{
    uint32_t decoded;
    size_t consumed;
    cl_utf8_status status;

    if (!it || it->offset > it->size || (!it->data && it->size != 0u)) {
        return CL_UTF8_INVALID;
    }
    if (it->offset == it->size) {
        return CL_UTF8_END;
    }

    if (byte_offset) {
        *byte_offset = it->offset;
    }

    status = cl_utf8_decode(
        it->data + it->offset,
        it->size - it->offset,
        &decoded,
        &consumed);
    if (status != CL_UTF8_OK) {
        return status;
    }

    it->offset += consumed;
    if (codepoint) {
        *codepoint = decoded;
    }
    return CL_UTF8_OK;
}
