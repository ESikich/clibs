/*
 * test_buffer.c
 * Purpose: Safety and behavior tests for cl_buffer.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_buffer.h"
#include "cl_test.h"

#include <string.h>

static int test_append_and_resize(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_buffer buffer;
    const unsigned char hello[] = { 'h', 'e', 'l', 'l', 'o' };

    cl_buffer_init(&buffer, &allocator);

    CL_TEST_CHECK(cl_buffer_append(&buffer, hello, sizeof(hello)));
    CL_TEST_CHECK(cl_buffer_append_byte(&buffer, (unsigned char)'!'));
    CL_TEST_CHECK(buffer.size == 6u);
    CL_TEST_CHECK(buffer.capacity >= buffer.size);
    CL_TEST_CHECK(memcmp(buffer.data, "hello!", 6u) == 0);

    CL_TEST_CHECK(cl_buffer_resize(&buffer, 9u));
    CL_TEST_CHECK(buffer.size == 9u);
    CL_TEST_CHECK(buffer.data[6] == 0u);
    CL_TEST_CHECK(buffer.data[8] == 0u);

    cl_buffer_clear(&buffer);
    CL_TEST_CHECK(buffer.size == 0u);
    CL_TEST_CHECK(buffer.capacity >= 9u);

    cl_buffer_free(&buffer);
    CL_TEST_CHECK(buffer.data == NULL);
    CL_TEST_CHECK(buffer.size == 0u);
    CL_TEST_CHECK(buffer.capacity == 0u);
    return 0;
}

static int test_append_rejects_overflow_without_mutating(void)
{
    cl_allocator allocator = cl_system_allocator();
    unsigned char byte = 7u;
    cl_buffer buffer;

    cl_buffer_init(&buffer, &allocator);
    buffer.data = &byte;
    buffer.size = (size_t)-2;
    buffer.capacity = (size_t)-1;

    CL_TEST_CHECK(!cl_buffer_append(&buffer, "abc", 3u));
    CL_TEST_CHECK(buffer.data == &byte);
    CL_TEST_CHECK(buffer.size == (size_t)-2);
    CL_TEST_CHECK(buffer.capacity == (size_t)-1);
    CL_TEST_CHECK(byte == 7u);

    buffer.data = NULL;
    buffer.size = 0u;
    buffer.capacity = 0u;
    return 0;
}

static int test_ring_push_pop_wraps(void)
{
    unsigned char storage[4];
    cl_ring_buffer ring;
    unsigned char out = 0u;

    cl_ring_buffer_init(&ring, storage, sizeof(storage));
    CL_TEST_CHECK(cl_ring_buffer_capacity(&ring) == 4u);
    CL_TEST_CHECK(cl_ring_buffer_available(&ring) == 4u);

    CL_TEST_CHECK(cl_ring_buffer_push(&ring, 1u));
    CL_TEST_CHECK(cl_ring_buffer_push(&ring, 2u));
    CL_TEST_CHECK(cl_ring_buffer_push(&ring, 3u));
    CL_TEST_CHECK(cl_ring_buffer_pop(&ring, &out));
    CL_TEST_CHECK(out == 1u);
    CL_TEST_CHECK(cl_ring_buffer_push(&ring, 4u));
    CL_TEST_CHECK(cl_ring_buffer_push(&ring, 5u));
    CL_TEST_CHECK(!cl_ring_buffer_push(&ring, 6u));

    CL_TEST_CHECK(cl_ring_buffer_pop(&ring, &out));
    CL_TEST_CHECK(out == 2u);
    CL_TEST_CHECK(cl_ring_buffer_pop(&ring, &out));
    CL_TEST_CHECK(out == 3u);
    CL_TEST_CHECK(cl_ring_buffer_pop(&ring, &out));
    CL_TEST_CHECK(out == 4u);
    CL_TEST_CHECK(cl_ring_buffer_pop(&ring, &out));
    CL_TEST_CHECK(out == 5u);
    CL_TEST_CHECK(!cl_ring_buffer_pop(&ring, &out));
    return 0;
}

static int test_ring_bulk_read_write(void)
{
    unsigned char storage[5];
    unsigned char out[5] = { 0u, 0u, 0u, 0u, 0u };
    cl_ring_buffer ring;

    cl_ring_buffer_init(&ring, storage, sizeof(storage));
    CL_TEST_CHECK(cl_ring_buffer_write(&ring, "abc", 3u) == 3u);
    CL_TEST_CHECK(cl_ring_buffer_read(&ring, out, 2u) == 2u);
    CL_TEST_CHECK(out[0] == (unsigned char)'a');
    CL_TEST_CHECK(out[1] == (unsigned char)'b');
    CL_TEST_CHECK(cl_ring_buffer_write(&ring, "defgh", 5u) == 4u);
    CL_TEST_CHECK(cl_ring_buffer_size(&ring) == 5u);
    CL_TEST_CHECK(cl_ring_buffer_available(&ring) == 0u);

    CL_TEST_CHECK(cl_ring_buffer_read(&ring, out, sizeof(out)) == 5u);
    CL_TEST_CHECK(memcmp(out, "cdefg", 5u) == 0);
    CL_TEST_CHECK(cl_ring_buffer_size(&ring) == 0u);

    cl_ring_buffer_reset(&ring);
    CL_TEST_CHECK(cl_ring_buffer_size(&ring) == 0u);
    return 0;
}

static int test_ring_null_storage_has_zero_capacity(void)
{
    cl_ring_buffer ring;

    cl_ring_buffer_init(&ring, NULL, 10u);
    CL_TEST_CHECK(cl_ring_buffer_capacity(&ring) == 0u);
    CL_TEST_CHECK(cl_ring_buffer_available(&ring) == 0u);
    CL_TEST_CHECK(!cl_ring_buffer_push(&ring, 1u));
    CL_TEST_CHECK(cl_ring_buffer_write(&ring, "x", 1u) == 0u);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_append_and_resize),
        CL_TEST_CASE(test_append_rejects_overflow_without_mutating),
        CL_TEST_CASE(test_ring_push_pop_wraps),
        CL_TEST_CASE(test_ring_bulk_read_write),
        CL_TEST_CASE(test_ring_null_storage_has_zero_capacity)
    };

    return cl_test_run_all("cl_buffer", cases, sizeof(cases) / sizeof(cases[0]));
}
