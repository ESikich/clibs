/*
 * test_path.c
 * Purpose: Safety and behavior tests for cl_path.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_path.h"
#include "cl_test.h"

#include <string.h>

static bool view_eq(cl_path_view view, const char *s)
{
    size_t size = strlen(s);

    return view.size == size && memcmp(view.data, s, size) == 0;
}

static int test_normalize_relative_paths(void)
{
    char out[64];
    size_t written = 0u;

    CL_TEST_CHECK(cl_path_normalize("a//b/./c/../d", out, sizeof(out), &written));
    CL_TEST_CHECK(strcmp(out, "a/b/d") == 0);
    CL_TEST_CHECK(written == 5u);

    CL_TEST_CHECK(cl_path_normalize("", out, sizeof(out), &written));
    CL_TEST_CHECK(strcmp(out, ".") == 0);
    CL_TEST_CHECK(written == 1u);

    CL_TEST_CHECK(cl_path_normalize("../../a/../b", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "../../b") == 0);
    return 0;
}

static int test_normalize_absolute_paths(void)
{
    char out[64];

    CL_TEST_CHECK(cl_path_normalize("/a//b/.././c/", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "/a/c") == 0);

    CL_TEST_CHECK(cl_path_normalize("/../../a", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "/a") == 0);

    CL_TEST_CHECK(cl_path_normalize("////", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "/") == 0);
    return 0;
}

static int test_join_paths(void)
{
    char out[64];
    size_t written = 0u;

    CL_TEST_CHECK(cl_path_join("/tmp/base", "../file.txt", out, sizeof(out), &written));
    CL_TEST_CHECK(strcmp(out, "/tmp/file.txt") == 0);
    CL_TEST_CHECK(written == 13u);

    CL_TEST_CHECK(cl_path_join("alpha/", "beta/./gamma", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "alpha/beta/gamma") == 0);

    CL_TEST_CHECK(cl_path_join("/ignored", "/absolute/child", out, sizeof(out), NULL));
    CL_TEST_CHECK(strcmp(out, "/absolute/child") == 0);
    return 0;
}

static int test_basename_and_dirname_views(void)
{
    CL_TEST_CHECK(view_eq(cl_path_basename("/tmp/name.txt"), "name.txt"));
    CL_TEST_CHECK(view_eq(cl_path_dirname("/tmp/name.txt"), "/tmp"));
    CL_TEST_CHECK(view_eq(cl_path_basename("alpha/"), "alpha"));
    CL_TEST_CHECK(view_eq(cl_path_dirname("alpha/"), "."));
    CL_TEST_CHECK(view_eq(cl_path_basename("/"), "/"));
    CL_TEST_CHECK(view_eq(cl_path_dirname("/"), "/"));
    CL_TEST_CHECK(view_eq(cl_path_basename(""), "."));
    CL_TEST_CHECK(view_eq(cl_path_dirname(""), "."));
    return 0;
}

static int test_rejects_invalid_or_too_small_outputs(void)
{
    char out[4];

    CL_TEST_CHECK(!cl_path_normalize(NULL, out, sizeof(out), NULL));
    CL_TEST_CHECK(!cl_path_normalize("abc", NULL, sizeof(out), NULL));
    CL_TEST_CHECK(!cl_path_normalize("abc", out, 0u, NULL));
    CL_TEST_CHECK(!cl_path_normalize("abcd", out, sizeof(out), NULL));
    CL_TEST_CHECK(!cl_path_join(NULL, "x", out, sizeof(out), NULL));
    CL_TEST_CHECK(!cl_path_join("x", NULL, out, sizeof(out), NULL));
    CL_TEST_CHECK(!cl_path_join("ab", "cd", out, sizeof(out), NULL));
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_normalize_relative_paths),
        CL_TEST_CASE(test_normalize_absolute_paths),
        CL_TEST_CASE(test_join_paths),
        CL_TEST_CASE(test_basename_and_dirname_views),
        CL_TEST_CASE(test_rejects_invalid_or_too_small_outputs)
    };

    return cl_test_run_all("cl_path", cases, sizeof(cases) / sizeof(cases[0]));
}
