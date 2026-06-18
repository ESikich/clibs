/*
 * test_heap.c
 * Purpose: Safety and behavior tests for cl_heap.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_heap.h"
#include "cl_test.h"

typedef struct heap_item {
    int id;
    int priority;
} heap_item;

static int compare_int_max(const void *left, const void *right, void *user)
{
    const int *a = (const int *)left;
    const int *b = (const int *)right;

    (void)user;
    if (*a > *b) {
        return 1;
    }
    if (*a < *b) {
        return -1;
    }
    return 0;
}

static int compare_int_min(const void *left, const void *right, void *user)
{
    return -compare_int_max(left, right, user);
}

static int compare_item_priority(
    const void *left,
    const void *right,
    void *user)
{
    const heap_item *a = (const heap_item *)left;
    const heap_item *b = (const heap_item *)right;
    int *bias = (int *)user;
    int left_priority = a->priority + (bias ? *bias : 0);
    int right_priority = b->priority + (bias ? *bias : 0);

    if (left_priority > right_priority) {
        return 1;
    }
    if (left_priority < right_priority) {
        return -1;
    }
    return 0;
}

static int test_make_and_pop_existing_array(void)
{
    heap_item items[] = {
        {1, 40},
        {2, 10},
        {3, 90},
        {4, 20},
        {5, 70}
    };
    int bias = 2;
    size_t count = sizeof(items) / sizeof(items[0]);

    CL_TEST_CHECK(cl_heap_make(
        items,
        count,
        sizeof(items[0]),
        compare_item_priority,
        &bias));
    CL_TEST_CHECK(cl_heap_is_valid(
        items,
        count,
        sizeof(items[0]),
        compare_item_priority,
        &bias));

    CL_TEST_CHECK(cl_heap_pop(
        items,
        count,
        sizeof(items[0]),
        compare_item_priority,
        &bias));
    --count;
    CL_TEST_CHECK(items[count].id == 3);
    CL_TEST_CHECK(cl_heap_is_valid(
        items,
        count,
        sizeof(items[0]),
        compare_item_priority,
        &bias));

    CL_TEST_CHECK(cl_heap_pop(
        items,
        count,
        sizeof(items[0]),
        compare_item_priority,
        &bias));
    --count;
    CL_TEST_CHECK(items[count].id == 5);

    return 0;
}

static int test_push_appended_slot(void)
{
    int values[5];
    size_t count = 0u;

    values[count++] = 5;
    CL_TEST_CHECK(cl_heap_push(
        values, count, sizeof(values[0]), compare_int_max, NULL));
    values[count++] = 2;
    CL_TEST_CHECK(cl_heap_push(
        values, count, sizeof(values[0]), compare_int_max, NULL));
    values[count++] = 9;
    CL_TEST_CHECK(cl_heap_push(
        values, count, sizeof(values[0]), compare_int_max, NULL));
    values[count++] = 1;
    CL_TEST_CHECK(cl_heap_push(
        values, count, sizeof(values[0]), compare_int_max, NULL));
    values[count++] = 7;
    CL_TEST_CHECK(cl_heap_push(
        values, count, sizeof(values[0]), compare_int_max, NULL));

    CL_TEST_CHECK(values[0] == 9);
    CL_TEST_CHECK(cl_heap_is_valid(
        values, count, sizeof(values[0]), compare_int_max, NULL));

    CL_TEST_CHECK(cl_heap_pop(
        values, count, sizeof(values[0]), compare_int_max, NULL));
    --count;
    CL_TEST_CHECK(values[count] == 9);
    CL_TEST_CHECK(values[0] == 7);

    return 0;
}

static int test_min_heap_and_sort(void)
{
    int values[] = {4, 1, 8, 3, 2};

    CL_TEST_CHECK(cl_heap_make(
        values,
        sizeof(values) / sizeof(values[0]),
        sizeof(values[0]),
        compare_int_min,
        NULL));
    CL_TEST_CHECK(values[0] == 1);

    CL_TEST_CHECK(cl_heap_sort(
        values,
        sizeof(values) / sizeof(values[0]),
        sizeof(values[0]),
        compare_int_max,
        NULL));
    CL_TEST_CHECK(values[0] == 1);
    CL_TEST_CHECK(values[1] == 2);
    CL_TEST_CHECK(values[2] == 3);
    CL_TEST_CHECK(values[3] == 4);
    CL_TEST_CHECK(values[4] == 8);

    CL_TEST_CHECK(cl_heap_sort(
        values,
        sizeof(values) / sizeof(values[0]),
        sizeof(values[0]),
        compare_int_min,
        NULL));
    CL_TEST_CHECK(values[0] == 8);
    CL_TEST_CHECK(values[1] == 4);
    CL_TEST_CHECK(values[2] == 3);
    CL_TEST_CHECK(values[3] == 2);
    CL_TEST_CHECK(values[4] == 1);

    return 0;
}

static int test_validation_rejects_invalid_inputs(void)
{
    int values[] = {1, 10, 5};

    CL_TEST_CHECK(!cl_heap_make(NULL, 1u, sizeof(values[0]), compare_int_max, NULL));
    CL_TEST_CHECK(!cl_heap_make(values, 3u, 0u, compare_int_max, NULL));
    CL_TEST_CHECK(!cl_heap_make(values, 3u, sizeof(values[0]), NULL, NULL));
    CL_TEST_CHECK(!cl_heap_push(values, 0u, sizeof(values[0]), compare_int_max, NULL));
    CL_TEST_CHECK(!cl_heap_pop(values, 0u, sizeof(values[0]), compare_int_max, NULL));
    CL_TEST_CHECK(cl_heap_make(NULL, 0u, sizeof(values[0]), compare_int_max, NULL));
    CL_TEST_CHECK(cl_heap_is_valid(NULL, 0u, sizeof(values[0]), compare_int_max, NULL));

    CL_TEST_CHECK(!cl_heap_is_valid(
        values,
        sizeof(values) / sizeof(values[0]),
        sizeof(values[0]),
        compare_int_max,
        NULL));

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_make_and_pop_existing_array),
        CL_TEST_CASE(test_push_appended_slot),
        CL_TEST_CASE(test_min_heap_and_sort),
        CL_TEST_CASE(test_validation_rejects_invalid_inputs)
    };

    return cl_test_run_all(
        "cl_heap",
        cases,
        sizeof(cases) / sizeof(cases[0]));
}
