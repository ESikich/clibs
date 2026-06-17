/*
 * test_array.c
 * Purpose: Safety and behavior tests for cl_array.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_array.h"
#include "cl_test.h"

#include <stdint.h>

CL_ARRAY_DEFINE(int_array, int)

typedef struct pair {
    int a;
    int b;
} pair;

CL_ARRAY_DEFINE(pair_array, pair)

static int test_push_and_pop(void)
{
    cl_allocator allocator = cl_system_allocator();
    int_array array;
    int value = 0;
    size_t i;

    int_array_init(&array, &allocator);

    for (i = 0u; i < 32u; ++i) {
        CL_TEST_CHECK(int_array_push(&array, (int)i));
    }

    CL_TEST_CHECK(array.size == 32u);
    CL_TEST_CHECK(array.capacity >= array.size);
    CL_TEST_CHECK(array.data[0] == 0);
    CL_TEST_CHECK(array.data[31] == 31);

    CL_TEST_CHECK(int_array_pop(&array, &value));
    CL_TEST_CHECK(value == 31);
    CL_TEST_CHECK(array.size == 31u);

    int_array_free(&array);
    CL_TEST_CHECK(array.data == NULL);
    CL_TEST_CHECK(array.size == 0u);
    CL_TEST_CHECK(array.capacity == 0u);

    return 0;
}

static int test_resize_zero_initializes_new_items(void)
{
    cl_allocator allocator = cl_system_allocator();
    pair_array array;

    pair_array_init(&array, &allocator);

    CL_TEST_CHECK(pair_array_resize(&array, 3u));
    CL_TEST_CHECK(array.size == 3u);
    CL_TEST_CHECK(array.data[0].a == 0);
    CL_TEST_CHECK(array.data[0].b == 0);
    CL_TEST_CHECK(array.data[2].a == 0);
    CL_TEST_CHECK(array.data[2].b == 0);

    array.data[1].a = 10;
    array.data[1].b = 20;
    CL_TEST_CHECK(pair_array_resize(&array, 1u));
    CL_TEST_CHECK(pair_array_resize(&array, 2u));
    CL_TEST_CHECK(array.data[0].a == 0);
    CL_TEST_CHECK(array.data[1].a == 0);
    CL_TEST_CHECK(array.data[1].b == 0);

    pair_array_free(&array);
    return 0;
}

static int test_arena_backed_array_respects_capacity(void)
{
    unsigned char storage[128];
    cl_arena arena;
    cl_allocator allocator;
    int_array array;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);
    int_array_init(&array, &allocator);

    CL_TEST_CHECK(int_array_reserve(&array, 8u));
    CL_TEST_CHECK(array.capacity >= 8u);
    CL_TEST_CHECK(int_array_push(&array, 7));
    CL_TEST_CHECK(array.data[0] == 7);
    CL_TEST_CHECK(!int_array_reserve(&array, 1000000u));
    CL_TEST_CHECK(array.data != NULL);
    CL_TEST_CHECK(array.size == 1u);

    int_array_free(&array);
    return 0;
}

static int test_checked_capacity_math(void)
{
    size_t out = 0u;

    CL_TEST_CHECK(cl_array_allocation_size(16u, sizeof(int), &out));
    CL_TEST_CHECK(out == 16u * sizeof(int));
    CL_TEST_CHECK(!cl_array_allocation_size((size_t)UINT64_MAX, 2u, &out));
    CL_TEST_CHECK(cl_array_next_capacity(0u, 1u, &out));
    CL_TEST_CHECK(out == 8u);
    CL_TEST_CHECK(cl_array_next_capacity(8u, 9u, &out));
    CL_TEST_CHECK(out >= 9u);

    return 0;
}

static int test_push_rejects_size_overflow(void)
{
    cl_allocator allocator = cl_system_allocator();
    int value = 3;
    int_array array;

    int_array_init(&array, &allocator);
    array.data = &value;
    array.size = (size_t)-1;
    array.capacity = (size_t)-1;

    CL_TEST_CHECK(!int_array_push(&array, 4));
    CL_TEST_CHECK(array.data == &value);
    CL_TEST_CHECK(array.size == (size_t)-1);
    CL_TEST_CHECK(value == 3);

    array.data = NULL;
    array.size = 0u;
    array.capacity = 0u;
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_push_and_pop),
        CL_TEST_CASE(test_resize_zero_initializes_new_items),
        CL_TEST_CASE(test_arena_backed_array_respects_capacity),
        CL_TEST_CASE(test_checked_capacity_math),
        CL_TEST_CASE(test_push_rejects_size_overflow)
    };

    return cl_test_run_all("cl_array", cases, sizeof(cases) / sizeof(cases[0]));
}
