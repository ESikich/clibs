/*
 * cl_ascii.c
 * Purpose: Implement locale-free ASCII classification and conversion helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_ascii.h"

bool cl_ascii_is_ascii(int c)
{
    return c >= 0 && c <= 0x7f;
}

bool cl_ascii_is_control(int c)
{
    return (c >= 0 && c <= 0x1f) || c == 0x7f;
}

bool cl_ascii_is_space(int c)
{
    return c == ' ' ||
           c == '\f' ||
           c == '\n' ||
           c == '\r' ||
           c == '\t' ||
           c == '\v';
}

bool cl_ascii_is_blank(int c)
{
    return c == ' ' || c == '\t';
}

bool cl_ascii_is_digit(int c)
{
    return c >= '0' && c <= '9';
}

bool cl_ascii_is_xdigit(int c)
{
    return cl_ascii_is_digit(c) ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

bool cl_ascii_is_lower(int c)
{
    return c >= 'a' && c <= 'z';
}

bool cl_ascii_is_upper(int c)
{
    return c >= 'A' && c <= 'Z';
}

bool cl_ascii_is_alpha(int c)
{
    return cl_ascii_is_lower(c) || cl_ascii_is_upper(c);
}

bool cl_ascii_is_alnum(int c)
{
    return cl_ascii_is_alpha(c) || cl_ascii_is_digit(c);
}

bool cl_ascii_is_print(int c)
{
    return c >= ' ' && c <= '~';
}

bool cl_ascii_is_graph(int c)
{
    return c >= '!' && c <= '~';
}

bool cl_ascii_is_punct(int c)
{
    return cl_ascii_is_graph(c) && !cl_ascii_is_alnum(c);
}

int cl_ascii_to_lower(int c)
{
    if (cl_ascii_is_upper(c)) {
        return c + ('a' - 'A');
    }
    return c;
}

int cl_ascii_to_upper(int c)
{
    if (cl_ascii_is_lower(c)) {
        return c - ('a' - 'A');
    }
    return c;
}

int cl_ascii_digit_value(int c)
{
    if (!cl_ascii_is_digit(c)) {
        return -1;
    }
    return c - '0';
}

int cl_ascii_hex_value(int c)
{
    if (cl_ascii_is_digit(c)) {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

bool cl_ascii_equal_ignore_case(int a, int b)
{
    return cl_ascii_to_lower(a) == cl_ascii_to_lower(b);
}
