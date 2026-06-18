/*
 * test_priority_queue.c
 * Purpose: Safety and behavior tests for cl_priority_queue.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_priority_queue.h"
#include "cl_test.h"

typedef struct priority_item {
    int id;
    int priority;
} priority_item;

static int compare_priority_desc(
    const void *left,
    const void *right,
    void *user)
{
    const priority_item *a = (const priority_item *)left;
    const priority_item *b = (const priority_item *)right;
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

static int compare_int_min_heap(const void *left, const void *right, void *user)
{
    const int *a = (const int *)left;
    const int *b = (const int *)right;

    (void)user;
    if (*a < *b) {
        return 1;
    }
    if (*a > *b) {
        return -1;
    }
    return 0;
}

static int test_push_pop_orders_highest_priority_first(void)
{
    priority_item storage[5];
    priority_item item;
    cl_priority_queue queue;
    int bias = 3;

    cl_priority_queue_init(
        &queue,
        storage,
        sizeof(storage),
        sizeof(storage[0]),
        compare_priority_desc,
        &bias);
    CL_TEST_CHECK(cl_priority_queue_capacity(&queue) == 5u);
    CL_TEST_CHECK(cl_priority_queue_available(&queue) == 5u);
    CL_TEST_CHECK(cl_priority_queue_is_empty(&queue));

    item.id = 1;
    item.priority = 40;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &item));
    item.id = 2;
    item.priority = 10;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &item));
    item.id = 3;
    item.priority = 90;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &item));
    item.id = 4;
    item.priority = 20;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &item));

    CL_TEST_CHECK(cl_priority_queue_size(&queue) == 4u);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 3);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 1);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 4);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 2);
    CL_TEST_CHECK(!cl_priority_queue_pop(&queue, &item));

    return 0;
}

static int test_min_heap_with_reversed_comparator(void)
{
    int storage[4];
    cl_priority_queue queue;
    int value;

    cl_priority_queue_init(
        &queue,
        storage,
        sizeof(storage),
        sizeof(storage[0]),
        compare_int_min_heap,
        NULL);

    value = 9;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    value = 2;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    value = 7;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    value = 1;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    CL_TEST_CHECK(cl_priority_queue_is_full(&queue));
    value = 0;
    CL_TEST_CHECK(!cl_priority_queue_push(&queue, &value));

    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 1);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 2);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 7);
    CL_TEST_CHECK(cl_priority_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 9);

    return 0;
}

static int test_peek_discard_and_reset(void)
{
    int storage[3];
    cl_priority_queue queue;
    int value;
    const int *peeked;

    cl_priority_queue_init(
        &queue,
        storage,
        sizeof(storage),
        sizeof(storage[0]),
        compare_int_min_heap,
        NULL);

    value = 3;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    value = 1;
    CL_TEST_CHECK(cl_priority_queue_push(&queue, &value));
    peeked = (const int *)cl_priority_queue_peek(&queue);
    CL_TEST_CHECK(peeked && *peeked == 1);

    CL_TEST_CHECK(cl_priority_queue_pop(&queue, NULL));
    peeked = (const int *)cl_priority_queue_peek(&queue);
    CL_TEST_CHECK(peeked && *peeked == 3);

    cl_priority_queue_reset(&queue);
    CL_TEST_CHECK(cl_priority_queue_is_empty(&queue));
    CL_TEST_CHECK(cl_priority_queue_available(&queue) == 3u);
    CL_TEST_CHECK(cl_priority_queue_peek(&queue) == NULL);

    return 0;
}

static int test_invalid_storage_or_comparator_is_empty_queue(void)
{
    unsigned char bytes[3];
    cl_priority_queue queue;
    int value = 1;

    cl_priority_queue_init(
        &queue,
        NULL,
        16u,
        sizeof(value),
        compare_int_min_heap,
        NULL);
    CL_TEST_CHECK(cl_priority_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(cl_priority_queue_element_size(&queue) == 0u);
    CL_TEST_CHECK(!cl_priority_queue_push(&queue, &value));

    cl_priority_queue_init(
        &queue,
        bytes,
        sizeof(bytes),
        sizeof(value),
        compare_int_min_heap,
        NULL);
    CL_TEST_CHECK(cl_priority_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(!cl_priority_queue_pop(&queue, &value));

    cl_priority_queue_init(
        &queue,
        bytes,
        sizeof(bytes),
        0u,
        compare_int_min_heap,
        NULL);
    CL_TEST_CHECK(cl_priority_queue_is_empty(&queue));

    cl_priority_queue_init(
        &queue,
        &value,
        sizeof(value),
        sizeof(value),
        NULL,
        NULL);
    CL_TEST_CHECK(cl_priority_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(cl_priority_queue_peek(&queue) == NULL);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_push_pop_orders_highest_priority_first),
        CL_TEST_CASE(test_min_heap_with_reversed_comparator),
        CL_TEST_CASE(test_peek_discard_and_reset),
        CL_TEST_CASE(test_invalid_storage_or_comparator_is_empty_queue)
    };

    return cl_test_run_all(
        "cl_priority_queue",
        cases,
        sizeof(cases) / sizeof(cases[0]));
}
