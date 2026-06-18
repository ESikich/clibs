/*
 * test_queue.c
 * Purpose: Safety and behavior tests for cl_queue.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_queue.h"
#include "cl_test.h"

typedef struct queue_item {
    int id;
    unsigned value;
} queue_item;

static int test_push_pop_and_wrap(void)
{
    queue_item storage[3];
    queue_item item;
    cl_queue queue;

    cl_queue_init(&queue, storage, sizeof(storage), sizeof(storage[0]));
    CL_TEST_CHECK(cl_queue_capacity(&queue) == 3u);
    CL_TEST_CHECK(cl_queue_available(&queue) == 3u);
    CL_TEST_CHECK(cl_queue_is_empty(&queue));

    item.id = 1;
    item.value = 10u;
    CL_TEST_CHECK(cl_queue_push(&queue, &item));
    item.id = 2;
    item.value = 20u;
    CL_TEST_CHECK(cl_queue_push(&queue, &item));

    CL_TEST_CHECK(cl_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 1);
    CL_TEST_CHECK(item.value == 10u);

    item.id = 3;
    item.value = 30u;
    CL_TEST_CHECK(cl_queue_push(&queue, &item));
    item.id = 4;
    item.value = 40u;
    CL_TEST_CHECK(cl_queue_push(&queue, &item));
    CL_TEST_CHECK(cl_queue_is_full(&queue));
    item.id = 5;
    item.value = 50u;
    CL_TEST_CHECK(!cl_queue_push(&queue, &item));

    CL_TEST_CHECK(cl_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 2);
    CL_TEST_CHECK(cl_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 3);
    CL_TEST_CHECK(cl_queue_pop(&queue, &item));
    CL_TEST_CHECK(item.id == 4);
    CL_TEST_CHECK(!cl_queue_pop(&queue, &item));
    CL_TEST_CHECK(cl_queue_is_empty(&queue));

    return 0;
}

static int test_front_peek_and_mutation(void)
{
    int storage[2];
    cl_queue queue;
    int value;
    const int *front;
    int *front_mut;

    cl_queue_init(&queue, storage, sizeof(storage), sizeof(storage[0]));
    value = 7;
    CL_TEST_CHECK(cl_queue_push(&queue, &value));
    value = 8;
    CL_TEST_CHECK(cl_queue_push(&queue, &value));

    front = (const int *)cl_queue_front(&queue);
    CL_TEST_CHECK(front && *front == 7);
    front_mut = (int *)cl_queue_front_mut(&queue);
    CL_TEST_CHECK(front_mut != NULL);
    *front_mut = 9;

    CL_TEST_CHECK(cl_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 9);
    CL_TEST_CHECK(cl_queue_size(&queue) == 1u);
    CL_TEST_CHECK(cl_queue_pop(&queue, &value));
    CL_TEST_CHECK(value == 8);

    return 0;
}

static int test_pop_can_discard_and_reset(void)
{
    int storage[2];
    cl_queue queue;
    int value = 11;

    cl_queue_init(&queue, storage, sizeof(storage), sizeof(storage[0]));
    CL_TEST_CHECK(cl_queue_push(&queue, &value));
    value = 12;
    CL_TEST_CHECK(cl_queue_push(&queue, &value));
    CL_TEST_CHECK(cl_queue_pop(&queue, NULL));
    CL_TEST_CHECK(cl_queue_size(&queue) == 1u);

    cl_queue_reset(&queue);
    CL_TEST_CHECK(cl_queue_is_empty(&queue));
    CL_TEST_CHECK(cl_queue_available(&queue) == 2u);
    CL_TEST_CHECK(cl_queue_front(&queue) == NULL);
    CL_TEST_CHECK(cl_queue_front_mut(&queue) == NULL);

    return 0;
}

static int test_invalid_storage_is_empty_queue(void)
{
    unsigned char bytes[3];
    cl_queue queue;
    int value = 1;

    cl_queue_init(&queue, NULL, 16u, sizeof(value));
    CL_TEST_CHECK(cl_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(cl_queue_element_size(&queue) == 0u);
    CL_TEST_CHECK(!cl_queue_push(&queue, &value));

    cl_queue_init(&queue, bytes, sizeof(bytes), sizeof(value));
    CL_TEST_CHECK(cl_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(!cl_queue_pop(&queue, &value));

    cl_queue_init(&queue, bytes, sizeof(bytes), 0u);
    CL_TEST_CHECK(cl_queue_capacity(&queue) == 0u);
    CL_TEST_CHECK(cl_queue_is_empty(&queue));

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_push_pop_and_wrap),
        CL_TEST_CASE(test_front_peek_and_mutation),
        CL_TEST_CASE(test_pop_can_discard_and_reset),
        CL_TEST_CASE(test_invalid_storage_is_empty_queue)
    };

    return cl_test_run_all("cl_queue", cases, sizeof(cases) / sizeof(cases[0]));
}
