/*
 * test_sv.c
 * Purpose: Safety and behavior tests for cl_sv.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_sv.h"
#include "cl_test.h"

#include <stdint.h>

static int test_constructors_and_validity(void)
{
    cl_sv empty = cl_sv_from_parts(NULL, 0u);
    cl_sv bad = cl_sv_from_parts(NULL, 3u);
    cl_sv text = cl_sv_from_cstr("alpha");

    CL_TEST_CHECK(empty.data == NULL);
    CL_TEST_CHECK(empty.size == 0u);
    CL_TEST_CHECK(bad.data == NULL);
    CL_TEST_CHECK(bad.size == 0u);
    CL_TEST_CHECK(text.data != NULL);
    CL_TEST_CHECK(text.size == 5u);
    CL_TEST_CHECK(cl_sv_is_valid(text));
    CL_TEST_CHECK(cl_sv_is_empty(empty));
    CL_TEST_CHECK(!cl_sv_is_valid((cl_sv){ NULL, 1u }));

    return 0;
}

static int test_trim(void)
{
    cl_sv text = cl_sv_from_cstr(" \t\nalpha beta\r\n");
    cl_sv trimmed = cl_sv_trim(text);
    cl_sv left = cl_sv_trim_left(text);
    cl_sv right = cl_sv_trim_right(text);

    CL_TEST_CHECK(cl_sv_eq(trimmed, cl_sv_from_cstr("alpha beta")));
    CL_TEST_CHECK(cl_sv_eq(left, cl_sv_from_cstr("alpha beta\r\n")));
    CL_TEST_CHECK(cl_sv_eq(right, cl_sv_from_cstr(" \t\nalpha beta")));
    CL_TEST_CHECK(cl_sv_trim((cl_sv){ NULL, 9u }).size == 0u);

    return 0;
}

static int test_compare_and_affixes(void)
{
    cl_sv alpha = cl_sv_from_cstr("alpha");

    CL_TEST_CHECK(cl_sv_eq(alpha, CL_SV_LIT("alpha")));
    CL_TEST_CHECK(!cl_sv_eq(alpha, CL_SV_LIT("alph")));
    CL_TEST_CHECK(cl_sv_cmp(CL_SV_LIT("abc"), CL_SV_LIT("abd")) < 0);
    CL_TEST_CHECK(cl_sv_cmp(CL_SV_LIT("abd"), CL_SV_LIT("abc")) > 0);
    CL_TEST_CHECK(cl_sv_cmp(CL_SV_LIT("abc"), CL_SV_LIT("abc")) == 0);
    CL_TEST_CHECK(cl_sv_cmp(CL_SV_LIT("abc"), CL_SV_LIT("abcd")) < 0);
    CL_TEST_CHECK(cl_sv_starts_with(alpha, CL_SV_LIT("al")));
    CL_TEST_CHECK(!cl_sv_starts_with(alpha, CL_SV_LIT("be")));
    CL_TEST_CHECK(cl_sv_ends_with(alpha, CL_SV_LIT("ha")));
    CL_TEST_CHECK(!cl_sv_ends_with(alpha, CL_SV_LIT("hp")));

    return 0;
}

static int test_split_once(void)
{
    cl_sv text = cl_sv_from_cstr("key=value=tail");
    cl_sv before;
    cl_sv after;

    CL_TEST_CHECK(cl_sv_split_once(text, '=', &before, &after));
    CL_TEST_CHECK(cl_sv_eq(before, CL_SV_LIT("key")));
    CL_TEST_CHECK(cl_sv_eq(after, CL_SV_LIT("value=tail")));
    CL_TEST_CHECK(!cl_sv_split_once(text, ':', &before, &after));

    return 0;
}

static int test_next_split_keeps_empty_fields(void)
{
    cl_sv rest = cl_sv_from_cstr("a,,b");
    cl_sv part;

    CL_TEST_CHECK(cl_sv_next_split(&rest, ',', &part));
    CL_TEST_CHECK(cl_sv_eq(part, CL_SV_LIT("a")));
    CL_TEST_CHECK(cl_sv_next_split(&rest, ',', &part));
    CL_TEST_CHECK(cl_sv_eq(part, CL_SV_LIT("")));
    CL_TEST_CHECK(cl_sv_next_split(&rest, ',', &part));
    CL_TEST_CHECK(cl_sv_eq(part, CL_SV_LIT("b")));
    CL_TEST_CHECK(!cl_sv_next_split(&rest, ',', &part));

    return 0;
}

static int test_parse_u64(void)
{
    uint64_t value = 7u;

    CL_TEST_CHECK(cl_sv_parse_u64(CL_SV_LIT(" 18446744073709551615 "), &value) ==
                  CL_SV_PARSE_OK);
    CL_TEST_CHECK(value == UINT64_MAX);
    CL_TEST_CHECK(cl_sv_parse_u64(CL_SV_LIT(""), &value) == CL_SV_PARSE_EMPTY);
    CL_TEST_CHECK(cl_sv_parse_u64(CL_SV_LIT("12x"), &value) ==
                  CL_SV_PARSE_INVALID);
    CL_TEST_CHECK(cl_sv_parse_u64((cl_sv){ NULL, 1u }, &value) ==
                  CL_SV_PARSE_INVALID);
    CL_TEST_CHECK(cl_sv_parse_u64(CL_SV_LIT("18446744073709551616"), &value) ==
                  CL_SV_PARSE_OVERFLOW);

    return 0;
}

static int test_parse_i64(void)
{
    int64_t value = 0;

    CL_TEST_CHECK(cl_sv_parse_i64(CL_SV_LIT("-9223372036854775808"), &value) ==
                  CL_SV_PARSE_OK);
    CL_TEST_CHECK(value == INT64_MIN);
    CL_TEST_CHECK(cl_sv_parse_i64(CL_SV_LIT("+9223372036854775807"), &value) ==
                  CL_SV_PARSE_OK);
    CL_TEST_CHECK(value == INT64_MAX);
    CL_TEST_CHECK(cl_sv_parse_i64(CL_SV_LIT("-9223372036854775809"), &value) ==
                  CL_SV_PARSE_OVERFLOW);
    CL_TEST_CHECK(cl_sv_parse_i64(CL_SV_LIT("+"), &value) == CL_SV_PARSE_INVALID);
    CL_TEST_CHECK(cl_sv_parse_i64(CL_SV_LIT("1 2"), &value) ==
                  CL_SV_PARSE_INVALID);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_constructors_and_validity),
        CL_TEST_CASE(test_trim),
        CL_TEST_CASE(test_compare_and_affixes),
        CL_TEST_CASE(test_split_once),
        CL_TEST_CASE(test_next_split_keeps_empty_fields),
        CL_TEST_CASE(test_parse_u64),
        CL_TEST_CASE(test_parse_i64)
    };

    return cl_test_run_all("cl_sv", cases, sizeof(cases) / sizeof(cases[0]));
}
