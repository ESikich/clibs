/*
 * cl_test.h
 * Purpose: Tiny unit test helpers for clibs tests.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_TEST_H
#define CL_TEST_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cl_test_fn)(void);

typedef struct cl_test_case {
    const char *name;
    cl_test_fn fn;
} cl_test_case;

static int cl_test_check_failed(
    const char *file,
    int line,
    const char *expr)
{
    fprintf(stderr, "FAIL %s:%d: %s\n", file, line, expr);
    return 1;
}

#define CL_TEST_CHECK(expr)                                                     \
    do {                                                                        \
        if (!(expr)) {                                                          \
            return cl_test_check_failed(__FILE__, __LINE__, #expr);             \
        }                                                                       \
    } while (0)

#define CL_TEST_CASE(fn) { #fn, fn }

static int cl_test_run_all(
    const char *suite_name,
    const cl_test_case *cases,
    size_t case_count)
{
    size_t passed = 0u;
    size_t failed = 0u;
    size_t i;

    if (!suite_name) {
        suite_name = "tests";
    }

    if (!cases && case_count != 0u) {
        fprintf(stderr, "FAIL %s: missing test cases\n", suite_name);
        return 1;
    }

    for (i = 0u; i < case_count; ++i) {
        int status;
        const char *name = cases[i].name ? cases[i].name : "(unnamed)";

        if (!cases[i].fn) {
            fprintf(stderr, "FAIL %s: %s has no function\n", suite_name, name);
            ++failed;
            continue;
        }

        status = cases[i].fn();
        if (status == 0) {
            ++passed;
        } else {
            fprintf(stderr, "FAIL %s: %s exited with %d\n",
                    suite_name, name, status);
            ++failed;
        }
    }

    if (failed == 0u) {
        printf("PASS %s: %zu tests\n", suite_name, passed);
        return 0;
    }

    fprintf(stderr, "FAIL %s: %zu passed, %zu failed\n",
            suite_name, passed, failed);
    return 1;
}

#ifdef __cplusplus
}
#endif

#endif
