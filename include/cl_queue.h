/*
 * cl_queue.h
 * Purpose: Allocation-free fixed-capacity FIFO ring queues for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_QUEUE_H
#define CL_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_queue {
    unsigned char *data;
    size_t storage_size;
    size_t element_size;
    size_t capacity;
    size_t head;
    size_t size;
} cl_queue;

void cl_queue_init(
    cl_queue *queue,
    void *storage,
    size_t storage_size,
    size_t element_size);
void cl_queue_reset(cl_queue *queue);

size_t cl_queue_size(const cl_queue *queue);
size_t cl_queue_capacity(const cl_queue *queue);
size_t cl_queue_available(const cl_queue *queue);
size_t cl_queue_element_size(const cl_queue *queue);
bool cl_queue_is_empty(const cl_queue *queue);
bool cl_queue_is_full(const cl_queue *queue);

bool cl_queue_push(cl_queue *queue, const void *element);
bool cl_queue_pop(cl_queue *queue, void *out);
const void *cl_queue_front(const cl_queue *queue);
void *cl_queue_front_mut(cl_queue *queue);

#ifdef __cplusplus
}
#endif

#endif
