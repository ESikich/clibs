/*
 * test_utf8.c
 * Purpose: Safety and behavior tests for cl_utf8.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_test.h"
#include "cl_utf8.h"

#include <stdint.h>
#include <string.h>

static int test_validate_and_count(void)
{
    const char text[] = "A\xC2\xA2\xE2\x82\xAC\xF0\x9F\x98\x80";

    CL_TEST_CHECK(cl_utf8_validate(text, sizeof(text) - 1u));
    CL_TEST_CHECK(cl_utf8_count_codepoints(text, sizeof(text) - 1u) == 4u);
    CL_TEST_CHECK(cl_utf8_validate(NULL, 0u));
    CL_TEST_CHECK(!cl_utf8_validate(NULL, 1u));

    return 0;
}

static int test_decode_valid_sequences(void)
{
    uint32_t codepoint = 0u;
    size_t consumed = 0u;

    CL_TEST_CHECK(cl_utf8_decode("A", 1u, &codepoint, &consumed) == CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0x41));
    CL_TEST_CHECK(consumed == 1u);

    CL_TEST_CHECK(cl_utf8_decode("\xC2\xA2", 2u, &codepoint, &consumed) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0xA2));
    CL_TEST_CHECK(consumed == 2u);

    CL_TEST_CHECK(cl_utf8_decode("\xE2\x82\xAC", 3u, &codepoint, &consumed) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0x20AC));
    CL_TEST_CHECK(consumed == 3u);

    CL_TEST_CHECK(cl_utf8_decode("\xF0\x9F\x98\x80", 4u, &codepoint, &consumed) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0x1F600));
    CL_TEST_CHECK(consumed == 4u);

    return 0;
}

static int test_decode_rejects_malformed_sequences(void)
{
    CL_TEST_CHECK(cl_utf8_decode("\xE2\x82", 2u, NULL, NULL) ==
                  CL_UTF8_TRUNCATED);
    CL_TEST_CHECK(cl_utf8_decode("\xC0\x80", 2u, NULL, NULL) ==
                  CL_UTF8_OVERLONG);
    CL_TEST_CHECK(cl_utf8_decode("\xE0\x80\x80", 3u, NULL, NULL) ==
                  CL_UTF8_OVERLONG);
    CL_TEST_CHECK(cl_utf8_decode("\xED\xA0\x80", 3u, NULL, NULL) ==
                  CL_UTF8_SURROGATE);
    CL_TEST_CHECK(cl_utf8_decode("\xF4\x90\x80\x80", 4u, NULL, NULL) ==
                  CL_UTF8_OUT_OF_RANGE);
    CL_TEST_CHECK(cl_utf8_decode("\x80", 1u, NULL, NULL) == CL_UTF8_INVALID);
    CL_TEST_CHECK(cl_utf8_decode("\xE2X\xAC", 3u, NULL, NULL) ==
                  CL_UTF8_INVALID);

    return 0;
}

static int test_encode(void)
{
    char out[CL_UTF8_MAX_BYTES];
    size_t written = 0u;

    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x24), out, sizeof(out), &written) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(written == 1u);
    CL_TEST_CHECK(memcmp(out, "\x24", written) == 0);

    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x20AC), out, sizeof(out), &written) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(written == 3u);
    CL_TEST_CHECK(memcmp(out, "\xE2\x82\xAC", written) == 0);

    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x1F600), out, sizeof(out), &written) ==
                  CL_UTF8_OK);
    CL_TEST_CHECK(written == 4u);
    CL_TEST_CHECK(memcmp(out, "\xF0\x9F\x98\x80", written) == 0);

    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0xD800), out, sizeof(out), NULL) ==
                  CL_UTF8_SURROGATE);
    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x110000), out, sizeof(out), NULL) ==
                  CL_UTF8_OUT_OF_RANGE);
    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x80), out, 1u, NULL) ==
                  CL_UTF8_TRUNCATED);
    CL_TEST_CHECK(cl_utf8_encode(UINT32_C(0x41), NULL, 1u, NULL) ==
                  CL_UTF8_INVALID);

    return 0;
}

static int test_iterator_reports_offsets_and_stops_on_error(void)
{
    const char text[] = "A\xE2\x82\xAC\x80";
    cl_utf8_iter it;
    uint32_t codepoint = 0u;
    size_t offset = 9u;

    cl_utf8_iter_init(&it, text, sizeof(text) - 1u);
    CL_TEST_CHECK(cl_utf8_iter_next(&it, &codepoint, &offset) == CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0x41));
    CL_TEST_CHECK(offset == 0u);
    CL_TEST_CHECK(it.offset == 1u);

    CL_TEST_CHECK(cl_utf8_iter_next(&it, &codepoint, &offset) == CL_UTF8_OK);
    CL_TEST_CHECK(codepoint == UINT32_C(0x20AC));
    CL_TEST_CHECK(offset == 1u);
    CL_TEST_CHECK(it.offset == 4u);

    CL_TEST_CHECK(cl_utf8_iter_next(&it, &codepoint, &offset) == CL_UTF8_INVALID);
    CL_TEST_CHECK(offset == 4u);
    CL_TEST_CHECK(it.offset == 4u);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_validate_and_count),
        CL_TEST_CASE(test_decode_valid_sequences),
        CL_TEST_CASE(test_decode_rejects_malformed_sequences),
        CL_TEST_CASE(test_encode),
        CL_TEST_CASE(test_iterator_reports_offsets_and_stops_on_error)
    };

    return cl_test_run_all("cl_utf8", cases, sizeof(cases) / sizeof(cases[0]));
}
