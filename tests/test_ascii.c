/*
 * test_ascii.c
 * Purpose: Safety and behavior tests for cl_ascii.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_ascii.h"
#include "cl_test.h"

static int test_classifies_core_ascii_ranges(void)
{
    CL_TEST_CHECK(cl_ascii_is_ascii(0));
    CL_TEST_CHECK(cl_ascii_is_ascii(0x7f));
    CL_TEST_CHECK(!cl_ascii_is_ascii(-1));
    CL_TEST_CHECK(!cl_ascii_is_ascii(0x80));

    CL_TEST_CHECK(cl_ascii_is_control('\n'));
    CL_TEST_CHECK(cl_ascii_is_control(0x7f));
    CL_TEST_CHECK(!cl_ascii_is_control('A'));

    CL_TEST_CHECK(cl_ascii_is_print(' '));
    CL_TEST_CHECK(cl_ascii_is_print('~'));
    CL_TEST_CHECK(!cl_ascii_is_print('\n'));
    CL_TEST_CHECK(!cl_ascii_is_print(0x80));

    CL_TEST_CHECK(cl_ascii_is_graph('!'));
    CL_TEST_CHECK(cl_ascii_is_graph('~'));
    CL_TEST_CHECK(!cl_ascii_is_graph(' '));

    return 0;
}

static int test_classifies_whitespace_and_alnum(void)
{
    CL_TEST_CHECK(cl_ascii_is_space(' '));
    CL_TEST_CHECK(cl_ascii_is_space('\t'));
    CL_TEST_CHECK(cl_ascii_is_space('\n'));
    CL_TEST_CHECK(cl_ascii_is_space('\r'));
    CL_TEST_CHECK(cl_ascii_is_space('\v'));
    CL_TEST_CHECK(cl_ascii_is_space('\f'));
    CL_TEST_CHECK(!cl_ascii_is_space('x'));
    CL_TEST_CHECK(!cl_ascii_is_space(0x85));

    CL_TEST_CHECK(cl_ascii_is_blank(' '));
    CL_TEST_CHECK(cl_ascii_is_blank('\t'));
    CL_TEST_CHECK(!cl_ascii_is_blank('\n'));

    CL_TEST_CHECK(cl_ascii_is_lower('a'));
    CL_TEST_CHECK(cl_ascii_is_lower('z'));
    CL_TEST_CHECK(!cl_ascii_is_lower('A'));
    CL_TEST_CHECK(cl_ascii_is_upper('A'));
    CL_TEST_CHECK(cl_ascii_is_upper('Z'));
    CL_TEST_CHECK(!cl_ascii_is_upper('z'));
    CL_TEST_CHECK(cl_ascii_is_alpha('q'));
    CL_TEST_CHECK(cl_ascii_is_alpha('Q'));
    CL_TEST_CHECK(!cl_ascii_is_alpha('3'));
    CL_TEST_CHECK(cl_ascii_is_alnum('3'));
    CL_TEST_CHECK(!cl_ascii_is_alnum('_'));

    return 0;
}

static int test_classifies_digits_hex_and_punctuation(void)
{
    CL_TEST_CHECK(cl_ascii_is_digit('0'));
    CL_TEST_CHECK(cl_ascii_is_digit('9'));
    CL_TEST_CHECK(!cl_ascii_is_digit('/'));
    CL_TEST_CHECK(!cl_ascii_is_digit(':'));

    CL_TEST_CHECK(cl_ascii_is_xdigit('0'));
    CL_TEST_CHECK(cl_ascii_is_xdigit('9'));
    CL_TEST_CHECK(cl_ascii_is_xdigit('a'));
    CL_TEST_CHECK(cl_ascii_is_xdigit('F'));
    CL_TEST_CHECK(!cl_ascii_is_xdigit('g'));
    CL_TEST_CHECK(!cl_ascii_is_xdigit('G'));

    CL_TEST_CHECK(cl_ascii_is_punct('!'));
    CL_TEST_CHECK(cl_ascii_is_punct('_'));
    CL_TEST_CHECK(!cl_ascii_is_punct('A'));
    CL_TEST_CHECK(!cl_ascii_is_punct(' '));

    return 0;
}

static int test_converts_case_without_locale(void)
{
    CL_TEST_CHECK(cl_ascii_to_lower('A') == 'a');
    CL_TEST_CHECK(cl_ascii_to_lower('Z') == 'z');
    CL_TEST_CHECK(cl_ascii_to_lower('a') == 'a');
    CL_TEST_CHECK(cl_ascii_to_lower('0') == '0');
    CL_TEST_CHECK(cl_ascii_to_lower(0xC0) == 0xC0);

    CL_TEST_CHECK(cl_ascii_to_upper('a') == 'A');
    CL_TEST_CHECK(cl_ascii_to_upper('z') == 'Z');
    CL_TEST_CHECK(cl_ascii_to_upper('A') == 'A');
    CL_TEST_CHECK(cl_ascii_to_upper('0') == '0');
    CL_TEST_CHECK(cl_ascii_to_upper(0xE0) == 0xE0);

    CL_TEST_CHECK(cl_ascii_equal_ignore_case('A', 'a'));
    CL_TEST_CHECK(cl_ascii_equal_ignore_case('x', 'X'));
    CL_TEST_CHECK(cl_ascii_equal_ignore_case('-', '-'));
    CL_TEST_CHECK(!cl_ascii_equal_ignore_case('a', 'b'));

    return 0;
}

static int test_digit_values(void)
{
    CL_TEST_CHECK(cl_ascii_digit_value('0') == 0);
    CL_TEST_CHECK(cl_ascii_digit_value('9') == 9);
    CL_TEST_CHECK(cl_ascii_digit_value('a') == -1);

    CL_TEST_CHECK(cl_ascii_hex_value('0') == 0);
    CL_TEST_CHECK(cl_ascii_hex_value('9') == 9);
    CL_TEST_CHECK(cl_ascii_hex_value('A') == 10);
    CL_TEST_CHECK(cl_ascii_hex_value('F') == 15);
    CL_TEST_CHECK(cl_ascii_hex_value('a') == 10);
    CL_TEST_CHECK(cl_ascii_hex_value('f') == 15);
    CL_TEST_CHECK(cl_ascii_hex_value('g') == -1);
    CL_TEST_CHECK(cl_ascii_hex_value(-1) == -1);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_classifies_core_ascii_ranges),
        CL_TEST_CASE(test_classifies_whitespace_and_alnum),
        CL_TEST_CASE(test_classifies_digits_hex_and_punctuation),
        CL_TEST_CASE(test_converts_case_without_locale),
        CL_TEST_CASE(test_digit_values)
    };

    return cl_test_run_all("cl_ascii", cases, sizeof(cases) / sizeof(cases[0]));
}
