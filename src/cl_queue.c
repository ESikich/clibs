/*
 * cl_queue.c
 * Purpose: Implement allocation-free fixed-capacity FIFO ring queues.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_queue.h"

#include <string.h>

static bool cl_queue_ready(const cl_queue *queue)
{
    return queue && queue->data && queue->element_size != 0u &&
           queue->capacity != 0u && queue->size <= queue->capacity;
}

static size_t cl_queue_tail_index(const cl_queue *queue)
{
    size_t tail;

    if (!cl_queue_ready(queue)) {
        return 0u;
    }

    if (queue->size >= queue->capacity - queue->head) {
        tail = queue->size - (queue->capacity - queue->head);
    } else {
        tail = queue->head + queue->size;
    }
    return tail;
}

static unsigned char *cl_queue_slot(cl_queue *queue, size_t index)
{
    return queue->data + (index * queue->element_size);
}

void cl_queue_init(
    cl_queue *queue,
    void *storage,
    size_t storage_size,
    size_t element_size)
{
    if (!queue) {
        return;
    }

    queue->data = (unsigned char *)storage;
    queue->storage_size = storage ? storage_size : 0u;
    queue->element_size = element_size;
    queue->capacity = 0u;
    queue->head = 0u;
    queue->size = 0u;

    if (!storage || storage_size == 0u || element_size == 0u) {
        queue->data = NULL;
        queue->storage_size = 0u;
        queue->element_size = 0u;
        return;
    }

    queue->capacity = storage_size / element_size;
    if (queue->capacity == 0u) {
        queue->data = NULL;
        queue->storage_size = 0u;
        queue->element_size = 0u;
    }
}

void cl_queue_reset(cl_queue *queue)
{
    if (!queue) {
        return;
    }

    queue->head = 0u;
    queue->size = 0u;
}

size_t cl_queue_size(const cl_queue *queue)
{
    return cl_queue_ready(queue) ? queue->size : 0u;
}

size_t cl_queue_capacity(const cl_queue *queue)
{
    return cl_queue_ready(queue) ? queue->capacity : 0u;
}

size_t cl_queue_available(const cl_queue *queue)
{
    if (!cl_queue_ready(queue)) {
        return 0u;
    }

    return queue->capacity - queue->size;
}

size_t cl_queue_element_size(const cl_queue *queue)
{
    return cl_queue_ready(queue) ? queue->element_size : 0u;
}

bool cl_queue_is_empty(const cl_queue *queue)
{
    return !cl_queue_ready(queue) || queue->size == 0u;
}

bool cl_queue_is_full(const cl_queue *queue)
{
    return cl_queue_ready(queue) && queue->size == queue->capacity;
}

bool cl_queue_push(cl_queue *queue, const void *element)
{
    size_t tail;

    if (!cl_queue_ready(queue) || !element || queue->size == queue->capacity) {
        return false;
    }

    tail = cl_queue_tail_index(queue);
    memcpy(cl_queue_slot(queue, tail), element, queue->element_size);
    ++queue->size;
    return true;
}

bool cl_queue_pop(cl_queue *queue, void *out)
{
    if (!cl_queue_ready(queue) || queue->size == 0u) {
        return false;
    }

    if (out) {
        memcpy(out, cl_queue_slot(queue, queue->head), queue->element_size);
    }

    ++queue->head;
    if (queue->head == queue->capacity) {
        queue->head = 0u;
    }
    --queue->size;
    return true;
}

const void *cl_queue_front(const cl_queue *queue)
{
    if (!cl_queue_ready(queue) || queue->size == 0u) {
        return NULL;
    }

    return queue->data + (queue->head * queue->element_size);
}

void *cl_queue_front_mut(cl_queue *queue)
{
    if (!cl_queue_ready(queue) || queue->size == 0u) {
        return NULL;
    }

    return cl_queue_slot(queue, queue->head);
}
