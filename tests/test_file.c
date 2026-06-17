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

static int test_stream_write_read_and_eof(void)
{
    const unsigned char bytes[] = { 'a', 'b', 'c' };
    unsigned char chunk[2];
    size_t nread;
    cl_file file;
    char path[128];

    test_path(path, sizeof(path), "stream.dat");
    (void)unlink(path);

    CL_TEST_CHECK(cl_file_open_write(path, &file));
    CL_TEST_CHECK(cl_file_write(&file, bytes, sizeof(bytes)));
    CL_TEST_CHECK(cl_file_close(&file));

    CL_TEST_CHECK(cl_file_open_read(path, &file));
    CL_TEST_CHECK(cl_file_read(&file, chunk, sizeof(chunk), &nread));
    CL_TEST_CHECK(nread == 2u);
    CL_TEST_CHECK(memcmp(chunk, "ab", 2u) == 0);
    CL_TEST_CHECK(cl_file_read(&file, chunk, sizeof(chunk), &nread));
    CL_TEST_CHECK(nread == 1u);
    CL_TEST_CHECK(chunk[0] == 'c');
    CL_TEST_CHECK(cl_file_read(&file, chunk, sizeof(chunk), &nread));
    CL_TEST_CHECK(nread == 0u);
    CL_TEST_CHECK(cl_file_close(&file));

    (void)unlink(path);
    return 0;
}

static int test_stream_append_preserves_existing_data(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_file_data file_data;
    cl_file file;
    char path[128];

    test_path(path, sizeof(path), "append.dat");
    (void)unlink(path);

    CL_TEST_CHECK(cl_file_open_write(path, &file));
    CL_TEST_CHECK(cl_file_write(&file, "first", 5u));
    CL_TEST_CHECK(cl_file_close(&file));

    CL_TEST_CHECK(cl_file_open_append(path, &file));
    CL_TEST_CHECK(cl_file_write(&file, "+second", 7u));
    CL_TEST_CHECK(cl_file_close(&file));

    CL_TEST_CHECK(cl_file_read_all(path, &allocator, &file_data));
    CL_TEST_CHECK(file_data.size == 12u);
    CL_TEST_CHECK(memcmp(file_data.data, "first+second", 12u) == 0);

    cl_file_data_free(&file_data);
    (void)unlink(path);
    return 0;
}

static int test_rejects_invalid_arguments(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_file_data file;
    cl_file stream;
    size_t nread = 1u;
    char path[128];

    test_path(path, sizeof(path), "invalid.dat");
    (void)unlink(path);
    cl_file_init(&stream);

    CL_TEST_CHECK(!cl_file_read_all(NULL, &allocator, &file));
    CL_TEST_CHECK(!cl_file_read_all(path, NULL, &file));
    CL_TEST_CHECK(!cl_file_read_all(path, &allocator, NULL));
    CL_TEST_CHECK(!cl_file_write_all(NULL, "x", 1u));
    CL_TEST_CHECK(!cl_file_write_all(path, NULL, 1u));
    CL_TEST_CHECK(!cl_file_open_read(NULL, &stream));
    CL_TEST_CHECK(!cl_file_open_write(NULL, &stream));
    CL_TEST_CHECK(!cl_file_open_append(NULL, &stream));
    CL_TEST_CHECK(!cl_file_read(NULL, &file, 1u, &nread));
    CL_TEST_CHECK(!cl_file_read(&stream, NULL, 1u, &nread));
    CL_TEST_CHECK(!cl_file_read(&stream, &file, 1u, NULL));
    CL_TEST_CHECK(!cl_file_write(NULL, "x", 1u));
    CL_TEST_CHECK(!cl_file_write(&stream, NULL, 1u));
    CL_TEST_CHECK(!cl_file_close(&stream));
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_write_and_read_binary_file),
        CL_TEST_CASE(test_empty_file_has_terminator),
        CL_TEST_CASE(test_read_missing_file_fails_cleanly),
        CL_TEST_CASE(test_stream_write_read_and_eof),
        CL_TEST_CASE(test_stream_append_preserves_existing_data),
        CL_TEST_CASE(test_rejects_invalid_arguments)
    };

    return cl_test_run_all("cl_file", cases, sizeof(cases) / sizeof(cases[0]));
}
