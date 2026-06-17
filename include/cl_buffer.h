/*
 * cl_buffer.h
 * Purpose: Growable byte buffers and caller-owned ring buffers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_BUFFER_H
#define CL_BUFFER_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_buffer {
    unsigned char *data;
    size_t size;
    size_t capacity;
    cl_allocator *allocator;
} cl_buffer;

void cl_buffer_init(cl_buffer *buffer, cl_allocator *allocator);
void cl_buffer_clear(cl_buffer *buffer);
void cl_buffer_free(cl_buffer *buffer);
bool cl_buffer_reserve(cl_buffer *buffer, size_t min_capacity);
bool cl_buffer_resize(cl_buffer *buffer, size_t new_size);
bool cl_buffer_append(cl_buffer *buffer, const void *data, size_t size);
bool cl_buffer_append_byte(cl_buffer *buffer, unsigned char byte);

typedef struct cl_ring_buffer {
    unsigned char *data;
    size_t capacity;
    size_t head;
    size_t size;
} cl_ring_buffer;

void cl_ring_buffer_init(cl_ring_buffer *ring, void *storage, size_t capacity);
void cl_ring_buffer_reset(cl_ring_buffer *ring);
size_t cl_ring_buffer_size(const cl_ring_buffer *ring);
size_t cl_ring_buffer_capacity(const cl_ring_buffer *ring);
size_t cl_ring_buffer_available(const cl_ring_buffer *ring);
bool cl_ring_buffer_push(cl_ring_buffer *ring, unsigned char byte);
bool cl_ring_buffer_pop(cl_ring_buffer *ring, unsigned char *out);
size_t cl_ring_buffer_write(cl_ring_buffer *ring, const void *data, size_t size);
size_t cl_ring_buffer_read(cl_ring_buffer *ring, void *out, size_t size);

#ifdef __cplusplus
}
#endif

#endif
