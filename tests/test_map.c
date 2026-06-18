/*
 * test_map.c
 * Purpose: Safety and behavior tests for cl_map.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_map.h"
#include "cl_test.h"

#include <string.h>

static int test_put_get_update_and_remove(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_map map;
    int apples = 4;
    int pears = 7;
    int replacement = 9;
    void *found = NULL;

    cl_map_init(&map, &allocator);
    CL_TEST_CHECK(cl_map_is_empty(&map));
    CL_TEST_CHECK(cl_map_put_cstr(&map, "apples", &apples));
    CL_TEST_CHECK(cl_map_put_cstr(&map, "pears", &pears));
    CL_TEST_CHECK(cl_map_size(&map) == 2u);

    CL_TEST_CHECK(cl_map_get_cstr(&map, "pears", &found));
    CL_TEST_CHECK(found == &pears);
    CL_TEST_CHECK(cl_map_put_cstr(&map, "pears", &replacement));
    CL_TEST_CHECK(cl_map_size(&map) == 2u);
    CL_TEST_CHECK(cl_map_get_cstr(&map, "pears", &found));
    CL_TEST_CHECK(found == &replacement);

    found = NULL;
    CL_TEST_CHECK(cl_map_remove_cstr(&map, "apples", &found));
    CL_TEST_CHECK(found == &apples);
    CL_TEST_CHECK(!cl_map_contains_cstr(&map, "apples"));
    CL_TEST_CHECK(cl_map_size(&map) == 1u);
    CL_TEST_CHECK(!cl_map_remove_cstr(&map, "missing", NULL));

    cl_map_free(&map);
    return 0;
}

static int test_iteration_is_lexicographic(void)
{
    cl_allocator allocator = cl_system_allocator();
    const char *keys[] = {"delta", "alpha", "charlie", "bravo"};
    const char *expected[] = {"alpha", "bravo", "charlie", "delta"};
    cl_map map;
    cl_map_iter iter;
    size_t i = 0u;

    cl_map_init(&map, &allocator);
    for (i = 0u; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        CL_TEST_CHECK(cl_map_put_cstr(&map, keys[i], (void *)keys[i]));
    }

    i = 0u;
    for (iter = cl_map_first(&map);
         cl_map_iter_is_valid(iter);
         iter = cl_map_next(iter)) {
        size_t key_size = 0u;
        const char *key = (const char *)cl_map_iter_key(iter, &key_size);

        CL_TEST_CHECK(i < sizeof(expected) / sizeof(expected[0]));
        CL_TEST_CHECK(key_size == strlen(expected[i]));
        CL_TEST_CHECK(memcmp(key, expected[i], key_size) == 0);
        CL_TEST_CHECK(
            strcmp((const char *)cl_map_iter_value(iter), expected[i]) == 0);
        ++i;
    }
    CL_TEST_CHECK(i == sizeof(expected) / sizeof(expected[0]));

    iter = cl_map_last(&map);
    CL_TEST_CHECK(cl_map_iter_is_valid(iter));
    CL_TEST_CHECK(
        strcmp((const char *)cl_map_iter_value(iter), expected[3]) == 0);
    iter = cl_map_prev(iter);
    CL_TEST_CHECK(
        strcmp((const char *)cl_map_iter_value(iter), expected[2]) == 0);

    cl_map_free(&map);
    return 0;
}

static int test_byte_keys_and_embedded_zeroes(void)
{
    cl_allocator allocator = cl_system_allocator();
    const unsigned char key_a[] = {'a', '\0', 'x'};
    const unsigned char key_b[] = {'a', '\0', 'y'};
    int value_a = 1;
    int value_b = 2;
    void *found = NULL;
    cl_map map;

    cl_map_init(&map, &allocator);
    CL_TEST_CHECK(cl_map_put(&map, key_b, sizeof(key_b), &value_b));
    CL_TEST_CHECK(cl_map_put(&map, key_a, sizeof(key_a), &value_a));
    CL_TEST_CHECK(cl_map_get(&map, key_a, sizeof(key_a), &found));
    CL_TEST_CHECK(found == &value_a);
    CL_TEST_CHECK(cl_map_get(&map, key_b, sizeof(key_b), &found));
    CL_TEST_CHECK(found == &value_b);

    found = NULL;
    CL_TEST_CHECK(cl_map_remove(&map, key_b, sizeof(key_b), &found));
    CL_TEST_CHECK(found == &value_b);
    CL_TEST_CHECK(!cl_map_contains(&map, key_b, sizeof(key_b)));
    CL_TEST_CHECK(cl_map_contains(&map, key_a, sizeof(key_a)));

    cl_map_free(&map);
    return 0;
}

static int test_removal_preserves_order_after_rotations(void)
{
    cl_allocator allocator = cl_system_allocator();
    int values[32];
    cl_map map;
    cl_map_iter iter;
    size_t i;
    int previous = -1;

    cl_map_init(&map, &allocator);
    for (i = 0u; i < sizeof(values) / sizeof(values[0]); ++i) {
        values[i] = (int)i;
        CL_TEST_CHECK(cl_map_put(&map, &values[i], sizeof(values[i]), &values[i]));
    }

    for (i = 0u; i < sizeof(values) / sizeof(values[0]); i += 2u) {
        CL_TEST_CHECK(cl_map_remove(&map, &values[i], sizeof(values[i]), NULL));
    }

    for (iter = cl_map_first(&map);
         cl_map_iter_is_valid(iter);
         iter = cl_map_next(iter)) {
        const int *value = (const int *)cl_map_iter_value(iter);

        CL_TEST_CHECK(value != NULL);
        CL_TEST_CHECK((*value % 2) == 1);
        CL_TEST_CHECK(*value > previous);
        previous = *value;
    }
    CL_TEST_CHECK(cl_map_size(&map) == 16u);

    cl_map_free(&map);
    return 0;
}

static int test_invalid_inputs_are_rejected(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_map map;
    int value = 1;
    cl_map_iter iter;

    cl_map_init(&map, &allocator);
    CL_TEST_CHECK(!cl_map_put(&map, NULL, 1u, &value));
    CL_TEST_CHECK(!cl_map_get(&map, NULL, 1u, NULL));
    CL_TEST_CHECK(!cl_map_remove(&map, NULL, 1u, NULL));
    CL_TEST_CHECK(!cl_map_put_cstr(&map, NULL, &value));
    CL_TEST_CHECK(!cl_map_get_cstr(&map, NULL, NULL));
    CL_TEST_CHECK(!cl_map_remove_cstr(&map, NULL, NULL));
    CL_TEST_CHECK(cl_map_put(&map, NULL, 0u, &value));
    CL_TEST_CHECK(cl_map_contains(&map, NULL, 0u));

    iter = cl_map_first(NULL);
    CL_TEST_CHECK(!cl_map_iter_is_valid(iter));
    CL_TEST_CHECK(cl_map_iter_key(iter, NULL) == NULL);
    CL_TEST_CHECK(cl_map_iter_value(iter) == NULL);

    cl_map_clear(&map);
    CL_TEST_CHECK(cl_map_is_empty(&map));
    cl_map_free(&map);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_put_get_update_and_remove),
        CL_TEST_CASE(test_iteration_is_lexicographic),
        CL_TEST_CASE(test_byte_keys_and_embedded_zeroes),
        CL_TEST_CASE(test_removal_preserves_order_after_rotations),
        CL_TEST_CASE(test_invalid_inputs_are_rejected),
    };

    return cl_test_run_all("cl_map", cases, sizeof(cases) / sizeof(cases[0]));
}
