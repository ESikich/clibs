/*
 * test_bitset.c
 * Purpose: Safety and behavior tests for cl_bitset.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_bitset.h"
#include "cl_test.h"

static int test_required_bytes_and_fixed_storage(void)
{
    size_t storage[2];
    size_t bytes = 0u;
    cl_bitset set;
    bool value = true;

    CL_TEST_CHECK(cl_bitset_required_bytes(0u, &bytes));
    CL_TEST_CHECK(bytes == 0u);
    CL_TEST_CHECK(cl_bitset_required_bytes(65u, &bytes));
    CL_TEST_CHECK(bytes == sizeof(storage));
    CL_TEST_CHECK(!cl_bitset_required_bytes(1u, NULL));

    CL_TEST_CHECK(cl_bitset_init_with_storage(&set, storage, sizeof(storage), 65u));
    CL_TEST_CHECK(cl_bitset_size(&set) == 65u);
    CL_TEST_CHECK(cl_bitset_count(&set) == 0u);
    CL_TEST_CHECK(cl_bitset_get(&set, 64u, &value));
    CL_TEST_CHECK(!value);
    CL_TEST_CHECK(!cl_bitset_get(&set, 65u, &value));
    CL_TEST_CHECK(!cl_bitset_set(&set, 65u));

    CL_TEST_CHECK(!cl_bitset_init_with_storage(&set, storage, sizeof(size_t), 65u));
    CL_TEST_CHECK(!cl_bitset_init_with_storage(
        &set, ((unsigned char *)storage) + 1u, sizeof(storage) - 1u, 65u));
    return 0;
}

static int test_set_clear_toggle_and_count(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_bitset set;
    bool value = false;

    CL_TEST_CHECK(cl_bitset_init(&set, &allocator, 130u));
    CL_TEST_CHECK(cl_bitset_set(&set, 0u));
    CL_TEST_CHECK(cl_bitset_set(&set, 64u));
    CL_TEST_CHECK(cl_bitset_set(&set, 129u));
    CL_TEST_CHECK(cl_bitset_count(&set) == 3u);

    CL_TEST_CHECK(cl_bitset_get(&set, 64u, &value));
    CL_TEST_CHECK(value);
    CL_TEST_CHECK(cl_bitset_clear(&set, 64u));
    CL_TEST_CHECK(cl_bitset_get(&set, 64u, &value));
    CL_TEST_CHECK(!value);
    CL_TEST_CHECK(cl_bitset_count(&set) == 2u);

    CL_TEST_CHECK(cl_bitset_toggle(&set, 1u));
    CL_TEST_CHECK(cl_bitset_toggle(&set, 129u));
    CL_TEST_CHECK(cl_bitset_count(&set) == 2u);
    CL_TEST_CHECK(cl_bitset_get(&set, 1u, &value));
    CL_TEST_CHECK(value);
    CL_TEST_CHECK(cl_bitset_get(&set, 129u, &value));
    CL_TEST_CHECK(!value);

    cl_bitset_clear_all(&set);
    CL_TEST_CHECK(cl_bitset_count(&set) == 0u);
    cl_bitset_fill_all(&set);
    CL_TEST_CHECK(cl_bitset_count(&set) == 130u);

    cl_bitset_free(&set);
    CL_TEST_CHECK(cl_bitset_size(&set) == 0u);
    return 0;
}

static int test_scans_find_set_and_clear_bits(void)
{
    size_t storage[3];
    cl_bitset set;
    size_t index = 99u;

    CL_TEST_CHECK(cl_bitset_init_with_storage(&set, storage, sizeof(storage), 129u));
    CL_TEST_CHECK(!cl_bitset_find_first_set(&set, 0u, &index));

    CL_TEST_CHECK(cl_bitset_set(&set, 5u));
    CL_TEST_CHECK(cl_bitset_set(&set, 70u));
    CL_TEST_CHECK(cl_bitset_set(&set, 128u));
    CL_TEST_CHECK(cl_bitset_find_first_set(&set, 0u, &index));
    CL_TEST_CHECK(index == 5u);
    CL_TEST_CHECK(cl_bitset_find_first_set(&set, 6u, &index));
    CL_TEST_CHECK(index == 70u);
    CL_TEST_CHECK(cl_bitset_find_first_set(&set, 71u, &index));
    CL_TEST_CHECK(index == 128u);
    CL_TEST_CHECK(!cl_bitset_find_first_set(&set, 129u, &index));

    cl_bitset_fill_all(&set);
    CL_TEST_CHECK(!cl_bitset_find_first_clear(&set, 0u, &index));
    CL_TEST_CHECK(cl_bitset_clear(&set, 127u));
    CL_TEST_CHECK(cl_bitset_find_first_clear(&set, 0u, &index));
    CL_TEST_CHECK(index == 127u);
    CL_TEST_CHECK(!cl_bitset_find_first_clear(&set, 128u, &index));

    return 0;
}

static int test_bit_operations_require_matching_sizes(void)
{
    size_t a_storage[2];
    size_t b_storage[2];
    size_t small_storage[1];
    cl_bitset a;
    cl_bitset b;
    cl_bitset small;
    bool value = false;

    CL_TEST_CHECK(cl_bitset_init_with_storage(&a, a_storage, sizeof(a_storage), 80u));
    CL_TEST_CHECK(cl_bitset_init_with_storage(&b, b_storage, sizeof(b_storage), 80u));
    CL_TEST_CHECK(cl_bitset_init_with_storage(
        &small, small_storage, sizeof(small_storage), 32u));

    CL_TEST_CHECK(cl_bitset_set(&a, 1u));
    CL_TEST_CHECK(cl_bitset_set(&a, 79u));
    CL_TEST_CHECK(cl_bitset_set(&b, 2u));
    CL_TEST_CHECK(cl_bitset_set(&b, 79u));

    CL_TEST_CHECK(cl_bitset_and(&a, &b));
    CL_TEST_CHECK(cl_bitset_count(&a) == 1u);
    CL_TEST_CHECK(cl_bitset_get(&a, 79u, &value));
    CL_TEST_CHECK(value);

    CL_TEST_CHECK(cl_bitset_or(&a, &b));
    CL_TEST_CHECK(cl_bitset_count(&a) == 2u);
    CL_TEST_CHECK(cl_bitset_get(&a, 2u, &value));
    CL_TEST_CHECK(value);

    CL_TEST_CHECK(cl_bitset_xor(&a, &b));
    CL_TEST_CHECK(cl_bitset_count(&a) == 0u);
    CL_TEST_CHECK(!cl_bitset_or(&a, &small));

    cl_bitset_not(&a);
    CL_TEST_CHECK(cl_bitset_count(&a) == 80u);
    CL_TEST_CHECK(cl_bitset_get(&a, 79u, &value));
    CL_TEST_CHECK(value);

    return 0;
}

static int test_zero_size_bitset_is_valid(void)
{
    cl_allocator allocator = cl_system_allocator();
    cl_bitset set;
    size_t index = 0u;
    bool value = false;

    CL_TEST_CHECK(cl_bitset_init(&set, &allocator, 0u));
    CL_TEST_CHECK(cl_bitset_size(&set) == 0u);
    CL_TEST_CHECK(cl_bitset_count(&set) == 0u);
    CL_TEST_CHECK(!cl_bitset_get(&set, 0u, &value));
    CL_TEST_CHECK(!cl_bitset_find_first_set(&set, 0u, &index));
    CL_TEST_CHECK(!cl_bitset_find_first_clear(&set, 0u, &index));
    CL_TEST_CHECK(cl_bitset_and(&set, &set));
    CL_TEST_CHECK(cl_bitset_or(&set, &set));
    CL_TEST_CHECK(cl_bitset_xor(&set, &set));
    cl_bitset_clear_all(&set);
    cl_bitset_fill_all(&set);
    cl_bitset_not(&set);
    cl_bitset_free(&set);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_required_bytes_and_fixed_storage),
        CL_TEST_CASE(test_set_clear_toggle_and_count),
        CL_TEST_CASE(test_scans_find_set_and_clear_bits),
        CL_TEST_CASE(test_bit_operations_require_matching_sizes),
        CL_TEST_CASE(test_zero_size_bitset_is_valid)
    };

    return cl_test_run_all("cl_bitset", cases, sizeof(cases) / sizeof(cases[0]));
}
