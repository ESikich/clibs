/*
 * test_libc.c
 * Purpose: Safety and behavior tests for cl_libc.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_libc.h"
#include "cl_test.h"

#include <stddef.h>

static int test_memset_sets_bytes(void)
{
    unsigned char bytes[8];
    size_t i;

    CL_TEST_CHECK(cl_memset(bytes, 0x5a, sizeof(bytes)) == bytes);
    for (i = 0u; i < sizeof(bytes); ++i) {
        CL_TEST_CHECK(bytes[i] == 0x5au);
    }
    CL_TEST_CHECK(cl_memset(NULL, 0, 0u) == NULL);
    CL_TEST_CHECK(cl_memset(NULL, 0, 1u) == NULL);

    return 0;
}

static int test_memcpy_copies_bytes(void)
{
    unsigned char src[5] = { 1u, 2u, 3u, 4u, 5u };
    unsigned char dst[5] = { 0u, 0u, 0u, 0u, 0u };
    size_t i;

    CL_TEST_CHECK(cl_memcpy(dst, src, sizeof(src)) == dst);
    for (i = 0u; i < sizeof(src); ++i) {
        CL_TEST_CHECK(dst[i] == src[i]);
    }
    CL_TEST_CHECK(cl_memcpy(NULL, src, 1u) == NULL);
    CL_TEST_CHECK(cl_memcpy(dst, NULL, 1u) == NULL);

    return 0;
}

static int test_memmove_handles_overlap(void)
{
    unsigned char right[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', '\0' };
    unsigned char left[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', '\0' };

    CL_TEST_CHECK(cl_memmove(right + 2u, right, 5u) == right + 2u);
    CL_TEST_CHECK(right[0] == 'a');
    CL_TEST_CHECK(right[1] == 'b');
    CL_TEST_CHECK(right[2] == 'a');
    CL_TEST_CHECK(right[3] == 'b');
    CL_TEST_CHECK(right[4] == 'c');
    CL_TEST_CHECK(right[5] == 'd');
    CL_TEST_CHECK(right[6] == 'e');

    CL_TEST_CHECK(cl_memmove(left, left + 2u, 5u) == left);
    CL_TEST_CHECK(left[0] == 'c');
    CL_TEST_CHECK(left[1] == 'd');
    CL_TEST_CHECK(left[2] == 'e');
    CL_TEST_CHECK(left[3] == 'f');
    CL_TEST_CHECK(left[4] == 'g');
    CL_TEST_CHECK(left[5] == 'f');

    return 0;
}

static int test_memcmp_orders_unsigned_bytes(void)
{
    unsigned char a[3] = { 0u, 0x80u, 2u };
    unsigned char b[3] = { 0u, 0x7fu, 3u };

    CL_TEST_CHECK(cl_memcmp(a, a, sizeof(a)) == 0);
    CL_TEST_CHECK(cl_memcmp(a, b, sizeof(a)) > 0);
    CL_TEST_CHECK(cl_memcmp(b, a, sizeof(a)) < 0);
    CL_TEST_CHECK(cl_memcmp(NULL, b, 1u) < 0);
    CL_TEST_CHECK(cl_memcmp(a, NULL, 1u) > 0);
    CL_TEST_CHECK(cl_memcmp(NULL, NULL, 0u) == 0);

    return 0;
}

static int test_string_functions(void)
{
    const char *text = "alpha";

    CL_TEST_CHECK(cl_strlen(text) == 5u);
    CL_TEST_CHECK(cl_strlen("") == 0u);
    CL_TEST_CHECK(cl_strlen(NULL) == 0u);

    CL_TEST_CHECK(cl_strcmp("abc", "abc") == 0);
    CL_TEST_CHECK(cl_strcmp("abc", "abd") < 0);
    CL_TEST_CHECK(cl_strcmp("abd", "abc") > 0);
    CL_TEST_CHECK(cl_strcmp("abc", "abcd") < 0);
    CL_TEST_CHECK(cl_strcmp(NULL, "abc") < 0);
    CL_TEST_CHECK(cl_strcmp("abc", NULL) > 0);

    CL_TEST_CHECK(cl_strchr(text, 'p') == text + 2u);
    CL_TEST_CHECK(cl_strchr(text, 'z') == NULL);
    CL_TEST_CHECK(cl_strchr(text, '\0') == text + 5u);
    CL_TEST_CHECK(cl_strchr(NULL, 'a') == NULL);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_memset_sets_bytes),
        CL_TEST_CASE(test_memcpy_copies_bytes),
        CL_TEST_CASE(test_memmove_handles_overlap),
        CL_TEST_CASE(test_memcmp_orders_unsigned_bytes),
        CL_TEST_CASE(test_string_functions)
    };

    return cl_test_run_all("cl_libc", cases, sizeof(cases) / sizeof(cases[0]));
}
