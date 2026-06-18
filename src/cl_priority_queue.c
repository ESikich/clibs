/*
 * cl_priority_queue.c
 * Purpose: Implement allocation-free fixed-capacity binary priority queues.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_priority_queue.h"

#include <string.h>

static bool cl_priority_queue_ready(const cl_priority_queue *queue)
{
    return queue && queue->data && queue->element_size != 0u &&
           queue->capacity != 0u && queue->size <= queue->capacity &&
           queue->compare;
}

static unsigned char *cl_priority_queue_slot(
    cl_priority_queue *queue,
    size_t index)
{
    return queue->data + (index * queue->element_size);
}

static const unsigned char *cl_priority_queue_const_slot(
    const cl_priority_queue *queue,
    size_t index)
{
    return queue->data + (index * queue->element_size);
}

static bool cl_priority_queue_higher(
    const cl_priority_queue *queue,
    size_t left,
    size_t right)
{
    return queue->compare(
               cl_priority_queue_const_slot(queue, left),
               cl_priority_queue_const_slot(queue, right),
               queue->user) > 0;
}

static void cl_priority_queue_swap(
    cl_priority_queue *queue,
    size_t left,
    size_t right)
{
    unsigned char *left_slot;
    unsigned char *right_slot;
    size_t i;

    if (left == right) {
        return;
    }

    left_slot = cl_priority_queue_slot(queue, left);
    right_slot = cl_priority_queue_slot(queue, right);
    for (i = 0u; i < queue->element_size; ++i) {
        unsigned char tmp = left_slot[i];
        left_slot[i] = right_slot[i];
        right_slot[i] = tmp;
    }
}

static void cl_priority_queue_sift_up(cl_priority_queue *queue, size_t index)
{
    while (index > 0u) {
        size_t parent = (index - 1u) / 2u;

        if (!cl_priority_queue_higher(queue, index, parent)) {
            return;
        }

        cl_priority_queue_swap(queue, index, parent);
        index = parent;
    }
}

static void cl_priority_queue_sift_down(cl_priority_queue *queue, size_t index)
{
    for (;;) {
        size_t left = (index * 2u) + 1u;
        size_t right = left + 1u;
        size_t highest = index;

        if (left < queue->size &&
            cl_priority_queue_higher(queue, left, highest)) {
            highest = left;
        }
        if (right < queue->size &&
            cl_priority_queue_higher(queue, right, highest)) {
            highest = right;
        }
        if (highest == index) {
            return;
        }

        cl_priority_queue_swap(queue, index, highest);
        index = highest;
    }
}

void cl_priority_queue_init(
    cl_priority_queue *queue,
    void *storage,
    size_t storage_size,
    size_t element_size,
    cl_priority_queue_compare compare,
    void *user)
{
    if (!queue) {
        return;
    }

    queue->data = (unsigned char *)storage;
    queue->storage_size = storage ? storage_size : 0u;
    queue->element_size = element_size;
    queue->capacity = 0u;
    queue->size = 0u;
    queue->compare = compare;
    queue->user = user;

    if (!storage || storage_size == 0u || element_size == 0u || !compare) {
        queue->data = NULL;
        queue->storage_size = 0u;
        queue->element_size = 0u;
        queue->compare = NULL;
        queue->user = NULL;
        return;
    }

    queue->capacity = storage_size / element_size;
    if (queue->capacity == 0u) {
        queue->data = NULL;
        queue->storage_size = 0u;
        queue->element_size = 0u;
        queue->compare = NULL;
        queue->user = NULL;
    }
}

void cl_priority_queue_reset(cl_priority_queue *queue)
{
    if (!queue) {
        return;
    }

    queue->size = 0u;
}

size_t cl_priority_queue_size(const cl_priority_queue *queue)
{
    return cl_priority_queue_ready(queue) ? queue->size : 0u;
}

size_t cl_priority_queue_capacity(const cl_priority_queue *queue)
{
    return cl_priority_queue_ready(queue) ? queue->capacity : 0u;
}

size_t cl_priority_queue_available(const cl_priority_queue *queue)
{
    if (!cl_priority_queue_ready(queue)) {
        return 0u;
    }

    return queue->capacity - queue->size;
}

size_t cl_priority_queue_element_size(const cl_priority_queue *queue)
{
    return cl_priority_queue_ready(queue) ? queue->element_size : 0u;
}

bool cl_priority_queue_is_empty(const cl_priority_queue *queue)
{
    return !cl_priority_queue_ready(queue) || queue->size == 0u;
}

bool cl_priority_queue_is_full(const cl_priority_queue *queue)
{
    return cl_priority_queue_ready(queue) && queue->size == queue->capacity;
}

bool cl_priority_queue_push(cl_priority_queue *queue, const void *element)
{
    if (!cl_priority_queue_ready(queue) || !element ||
        queue->size == queue->capacity) {
        return false;
    }

    memcpy(
        cl_priority_queue_slot(queue, queue->size),
        element,
        queue->element_size);
    cl_priority_queue_sift_up(queue, queue->size);
    ++queue->size;
    return true;
}

bool cl_priority_queue_pop(cl_priority_queue *queue, void *out)
{
    if (!cl_priority_queue_ready(queue) || queue->size == 0u) {
        return false;
    }

    if (out) {
        memcpy(out, cl_priority_queue_slot(queue, 0u), queue->element_size);
    }

    --queue->size;
    if (queue->size != 0u) {
        memcpy(
            cl_priority_queue_slot(queue, 0u),
            cl_priority_queue_slot(queue, queue->size),
            queue->element_size);
        cl_priority_queue_sift_down(queue, 0u);
    }
    return true;
}

const void *cl_priority_queue_peek(const cl_priority_queue *queue)
{
    if (!cl_priority_queue_ready(queue) || queue->size == 0u) {
        return NULL;
    }

    return cl_priority_queue_const_slot(queue, 0u);
}
