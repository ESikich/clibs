/*
 * test_hash.c
 * Purpose: Safety and behavior tests for cl_hash.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_hash.h"
#include "cl_test.h"

#include <stdint.h>

static int test_fnv1a_known_values(void)
{
    CL_TEST_CHECK(cl_hash_fnv1a64("", 0u) == 14695981039346656037ull);
    CL_TEST_CHECK(cl_hash_cstr("hello") == 11831194018420276491ull);
    CL_TEST_CHECK(cl_hash_fnv1a64(NULL, 1u) == 0u);
    CL_TEST_CHECK(cl_hash_cstr(NULL) == 0u);
    return 0;
}

static int test_put_get_and_update(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_hash_table table;
    int apples = 4;
    int oranges = 2;
    int next_apples = 8;
    void *value = NULL;

    cl_hash_table_init(&table, &allocator);

    CL_TEST_CHECK(cl_hash_table_put_cstr(&table, "apples", &apples));
    CL_TEST_CHECK(cl_hash_table_put_cstr(&table, "oranges", &oranges));
    CL_TEST_CHECK(cl_hash_table_size(&table) == 2u);
    CL_TEST_CHECK(cl_hash_table_get_cstr(&table, "apples", &value));
    CL_TEST_CHECK(*(int *)value == 4);

    CL_TEST_CHECK(cl_hash_table_put_cstr(&table, "apples", &next_apples));
    CL_TEST_CHECK(cl_hash_table_size(&table) == 2u);
    CL_TEST_CHECK(cl_hash_table_get_cstr(&table, "apples", &value));
    CL_TEST_CHECK(*(int *)value == 8);
    CL_TEST_CHECK(!cl_hash_table_get_cstr(&table, "pears", &value));

    cl_hash_table_free(&table);
    CL_TEST_CHECK(table.entries == NULL);
    CL_TEST_CHECK(table.size == 0u);
    CL_TEST_CHECK(table.capacity == 0u);
    return 0;
}

static int test_byte_keys_are_not_cstrings(void)
{
    cl_allocator allocator = cl_system_allocator();
    const unsigned char key_a[] = { 'a', 0, 'b' };
    const unsigned char key_b[] = { 'a', 0, 'c' };
    cl_hash_table table;
    int value_a = 11;
    int value_b = 22;
    void *value = NULL;

    cl_hash_table_init(&table, &allocator);

    CL_TEST_CHECK(cl_hash_table_put(&table, key_a, sizeof(key_a), &value_a));
    CL_TEST_CHECK(cl_hash_table_put(&table, key_b, sizeof(key_b), &value_b));
    CL_TEST_CHECK(cl_hash_table_get(&table, key_a, sizeof(key_a), &value));
    CL_TEST_CHECK(*(int *)value == 11);
    CL_TEST_CHECK(cl_hash_table_get(&table, key_b, sizeof(key_b), &value));
    CL_TEST_CHECK(*(int *)value == 22);

    cl_hash_table_free(&table);
    return 0;
}

static int test_remove_leaves_probe_chain_usable(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_hash_table table;
    int values[64];
    void *removed = NULL;
    void *value = NULL;
    size_t i;

    cl_hash_table_init(&table, &allocator);

    for (i = 0u; i < 64u; ++i) {
        values[i] = (int)i;
        CL_TEST_CHECK(cl_hash_table_put(&table, &values[i], sizeof(values[i]), &values[i]));
    }

    CL_TEST_CHECK(cl_hash_table_remove(&table, &values[10], sizeof(values[10]), &removed));
    CL_TEST_CHECK(*(int *)removed == 10);
    CL_TEST_CHECK(!cl_hash_table_contains(&table, &values[10], sizeof(values[10])));
    CL_TEST_CHECK(cl_hash_table_get(&table, &values[11], sizeof(values[11]), &value));
    CL_TEST_CHECK(*(int *)value == 11);
    CL_TEST_CHECK(cl_hash_table_put(&table, &values[10], sizeof(values[10]), &values[10]));
    CL_TEST_CHECK(cl_hash_table_get(&table, &values[10], sizeof(values[10]), &value));
    CL_TEST_CHECK(*(int *)value == 10);

    cl_hash_table_free(&table);
    return 0;
}

static int test_arena_backed_table_respects_capacity(void)
{
    unsigned char storage[1024];
    cl_arena arena;
    cl_allocator allocator;
    cl_hash_table table;
    int value = 1;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);
    cl_hash_table_init(&table, &allocator);

    CL_TEST_CHECK(!cl_hash_table_reserve(&table, 1000000u));
    CL_TEST_CHECK(table.entries == NULL);
    CL_TEST_CHECK(cl_hash_table_put_cstr(&table, "small", &value));
    CL_TEST_CHECK(!cl_hash_table_reserve(&table, 1000000u));
    CL_TEST_CHECK(cl_hash_table_contains_cstr(&table, "small"));

    cl_hash_table_free(&table);
    return 0;
}

static int test_clear_keeps_capacity(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_hash_table table;
    size_t capacity;
    int value = 5;

    cl_hash_table_init(&table, &allocator);
    CL_TEST_CHECK(cl_hash_table_put_cstr(&table, "x", &value));
    capacity = cl_hash_table_capacity(&table);
    CL_TEST_CHECK(capacity != 0u);

    cl_hash_table_clear(&table);
    CL_TEST_CHECK(cl_hash_table_size(&table) == 0u);
    CL_TEST_CHECK(cl_hash_table_capacity(&table) == capacity);
    CL_TEST_CHECK(!cl_hash_table_contains_cstr(&table, "x"));

    cl_hash_table_free(&table);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_fnv1a_known_values),
        CL_TEST_CASE(test_put_get_and_update),
        CL_TEST_CASE(test_byte_keys_are_not_cstrings),
        CL_TEST_CASE(test_remove_leaves_probe_chain_usable),
        CL_TEST_CASE(test_arena_backed_table_respects_capacity),
        CL_TEST_CASE(test_clear_keeps_capacity)
    };

    return cl_test_run_all("cl_hash", cases, sizeof(cases) / sizeof(cases[0]));
}
