/*
 * test_set.c
 * Purpose: Safety and behavior tests for cl_set.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_set.h"
#include "cl_test.h"

static int test_insert_contains_and_duplicate(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_set set;

    cl_set_init(&set, &allocator);

    CL_TEST_CHECK(cl_set_insert_cstr(&set, "alpha"));
    CL_TEST_CHECK(cl_set_insert_cstr(&set, "beta"));
    CL_TEST_CHECK(cl_set_size(&set) == 2u);
    CL_TEST_CHECK(cl_set_contains_cstr(&set, "alpha"));
    CL_TEST_CHECK(cl_set_contains_cstr(&set, "beta"));
    CL_TEST_CHECK(!cl_set_contains_cstr(&set, "gamma"));

    CL_TEST_CHECK(cl_set_insert_cstr(&set, "alpha"));
    CL_TEST_CHECK(cl_set_size(&set) == 2u);

    cl_set_free(&set);
    CL_TEST_CHECK(cl_set_size(&set) == 0u);
    CL_TEST_CHECK(cl_set_capacity(&set) == 0u);
    return 0;
}

static int test_byte_keys_are_not_cstrings(void)
{
    cl_allocator allocator = cl_system_allocator();
    const unsigned char key_a[] = { 'a', 0, 'b' };
    const unsigned char key_b[] = { 'a', 0, 'c' };
    cl_set set;

    cl_set_init(&set, &allocator);

    CL_TEST_CHECK(cl_set_insert(&set, key_a, sizeof(key_a)));
    CL_TEST_CHECK(cl_set_insert(&set, key_b, sizeof(key_b)));
    CL_TEST_CHECK(cl_set_contains(&set, key_a, sizeof(key_a)));
    CL_TEST_CHECK(cl_set_contains(&set, key_b, sizeof(key_b)));
    CL_TEST_CHECK(!cl_set_contains(&set, "a", 1u));

    cl_set_free(&set);
    return 0;
}

static int test_remove_and_reinsert(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_set set;
    int keys[64];
    size_t i;

    cl_set_init(&set, &allocator);

    for (i = 0u; i < 64u; ++i) {
        keys[i] = (int)i;
        CL_TEST_CHECK(cl_set_insert(&set, &keys[i], sizeof(keys[i])));
    }

    CL_TEST_CHECK(cl_set_remove(&set, &keys[10], sizeof(keys[10])));
    CL_TEST_CHECK(!cl_set_contains(&set, &keys[10], sizeof(keys[10])));
    CL_TEST_CHECK(cl_set_contains(&set, &keys[11], sizeof(keys[11])));
    CL_TEST_CHECK(!cl_set_remove(&set, &keys[10], sizeof(keys[10])));
    CL_TEST_CHECK(cl_set_insert(&set, &keys[10], sizeof(keys[10])));
    CL_TEST_CHECK(cl_set_contains(&set, &keys[10], sizeof(keys[10])));

    cl_set_free(&set);
    return 0;
}

static int test_clear_keeps_capacity(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_set set;
    size_t capacity;

    cl_set_init(&set, &allocator);
    CL_TEST_CHECK(cl_set_insert_cstr(&set, "x"));
    capacity = cl_set_capacity(&set);
    CL_TEST_CHECK(capacity != 0u);

    cl_set_clear(&set);
    CL_TEST_CHECK(cl_set_size(&set) == 0u);
    CL_TEST_CHECK(cl_set_capacity(&set) == capacity);
    CL_TEST_CHECK(!cl_set_contains_cstr(&set, "x"));

    cl_set_free(&set);
    return 0;
}

static int test_arena_backed_set_respects_capacity(void)
{
    unsigned char storage[1024];
    cl_arena arena;
    cl_allocator allocator;
    cl_set set;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);
    cl_set_init(&set, &allocator);

    CL_TEST_CHECK(!cl_set_reserve(&set, 1000000u));
    CL_TEST_CHECK(cl_set_capacity(&set) == 0u);
    CL_TEST_CHECK(cl_set_insert_cstr(&set, "small"));
    CL_TEST_CHECK(!cl_set_reserve(&set, 1000000u));
    CL_TEST_CHECK(cl_set_contains_cstr(&set, "small"));

    cl_set_free(&set);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_insert_contains_and_duplicate),
        CL_TEST_CASE(test_byte_keys_are_not_cstrings),
        CL_TEST_CASE(test_remove_and_reinsert),
        CL_TEST_CASE(test_clear_keeps_capacity),
        CL_TEST_CASE(test_arena_backed_set_respects_capacity)
    };

    return cl_test_run_all("cl_set", cases, sizeof(cases) / sizeof(cases[0]));
}
