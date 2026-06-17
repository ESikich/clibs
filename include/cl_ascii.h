/*
 * cl_ascii.h
 * Purpose: Locale-free ASCII classification and conversion helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_ASCII_H
#define CL_ASCII_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool cl_ascii_is_ascii(int c);
bool cl_ascii_is_control(int c);
bool cl_ascii_is_space(int c);
bool cl_ascii_is_blank(int c);
bool cl_ascii_is_digit(int c);
bool cl_ascii_is_xdigit(int c);
bool cl_ascii_is_lower(int c);
bool cl_ascii_is_upper(int c);
bool cl_ascii_is_alpha(int c);
bool cl_ascii_is_alnum(int c);
bool cl_ascii_is_print(int c);
bool cl_ascii_is_graph(int c);
bool cl_ascii_is_punct(int c);

int cl_ascii_to_lower(int c);
int cl_ascii_to_upper(int c);
int cl_ascii_digit_value(int c);
int cl_ascii_hex_value(int c);
bool cl_ascii_equal_ignore_case(int a, int b);

#ifdef __cplusplus
}
#endif

#endif
