/*
 * test_file.c
 * Purpose: Safety and behavior tests for cl_file.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "cl_file.h"
#include "cl_test.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_path(char *path, size_t path_size, const char *name)
{
    (void)snprintf(path, path_size, "/tmp/clibs_%ld_%s", (long)getpid(), name);
}

static int test_write_and_read_binary_file(void)
{
    cl_allocator allocator = cl_system_allocator();
    const unsigned char bytes[] = { 'a', 0u, 'b', '\n' };
    cl_file_data file;
    char path[128];

    test_path(path, sizeof(path), "binary.dat");
    (void)unlink(path);

    CL_TEST_CHECK(cl_file_write_all(path, bytes, sizeof(bytes)));
    CL_TEST_CHECK(cl_file_read_all(path, &allocator, &file));
    CL_TEST_CHECK(file.size == sizeof(bytes));
    CL_TEST_CHECK(file.capacity >= file.size + 1u);
    CL_TEST_CHECK(memcmp(file.data, bytes, sizeof(bytes)) == 0);
    CL_TEST_CHECK(file.data[file.size] == 0u);

    cl_file_data_free(&file);
    (void)unlink(path);
    return 0;
}

static int test_empty_file_has_terminator(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_file_data file;
    char path[128];

    test_path(path, sizeof(path), "empty.dat");
    (void)unlink(path);

    CL_TEST_CHECK(cl_file_write_all(path, NULL, 0u));
    CL_TEST_CHECK(cl_file_read_all(path, &allocator, &file));
    CL_TEST_CHECK(file.size == 0u);
    CL_TEST_CHECK(file.capacity == 1u);
    CL_TEST_CHECK(file.data[0] == 0u);

    cl_file_data_free(&file);
    (void)unlink(path);
    return 0;
}

static int test_read_missing_file_fails_cleanly(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_file_data file;
    char path[128];

    test_path(path, sizeof(path), "missing.dat");
    (void)unlink(path);
    cl_file_data_init(&file, &allocator);

    CL_TEST_CHECK(!cl_file_read_all(path, &allocator, &file));
    CL_TEST_CHECK(file.data == NULL);
    CL_TEST_CHECK(file.size == 0u);
    CL_TEST_CHECK(file.capacity == 0u);
    return 0;
}

static int test_rejects_invalid_arguments(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_file_data file;
    char path[128];

    test_path(path, sizeof(path), "invalid.dat");
    (void)unlink(path);

    CL_TEST_CHECK(!cl_file_read_all(NULL, &allocator, &file));
    CL_TEST_CHECK(!cl_file_read_all(path, NULL, &file));
    CL_TEST_CHECK(!cl_file_read_all(path, &allocator, NULL));
    CL_TEST_CHECK(!cl_file_write_all(NULL, "x", 1u));
    CL_TEST_CHECK(!cl_file_write_all(path, NULL, 1u));
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_write_and_read_binary_file),
        CL_TEST_CASE(test_empty_file_has_terminator),
        CL_TEST_CASE(test_read_missing_file_fails_cleanly),
        CL_TEST_CASE(test_rejects_invalid_arguments)
    };

    return cl_test_run_all("cl_file", cases, sizeof(cases) / sizeof(cases[0]));
}
