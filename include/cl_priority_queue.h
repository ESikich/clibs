/*
 * cl_priority_queue.h
 * Purpose: Allocation-free fixed-capacity binary priority queues for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_PRIORITY_QUEUE_H
#define CL_PRIORITY_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cl_priority_queue_compare)(
    const void *left,
    const void *right,
    void *user);

typedef struct cl_priority_queue {
    unsigned char *data;
    size_t storage_size;
    size_t element_size;
    size_t capacity;
    size_t size;
    cl_priority_queue_compare compare;
    void *user;
} cl_priority_queue;

void cl_priority_queue_init(
    cl_priority_queue *queue,
    void *storage,
    size_t storage_size,
    size_t element_size,
    cl_priority_queue_compare compare,
    void *user);
void cl_priority_queue_reset(cl_priority_queue *queue);

size_t cl_priority_queue_size(const cl_priority_queue *queue);
size_t cl_priority_queue_capacity(const cl_priority_queue *queue);
size_t cl_priority_queue_available(const cl_priority_queue *queue);
size_t cl_priority_queue_element_size(const cl_priority_queue *queue);
bool cl_priority_queue_is_empty(const cl_priority_queue *queue);
bool cl_priority_queue_is_full(const cl_priority_queue *queue);

bool cl_priority_queue_push(cl_priority_queue *queue, const void *element);
bool cl_priority_queue_pop(cl_priority_queue *queue, void *out);
const void *cl_priority_queue_peek(const cl_priority_queue *queue);

#ifdef __cplusplus
}
#endif

#endif
