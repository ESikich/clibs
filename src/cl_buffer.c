/*
 * cl_buffer.c
 * Purpose: Growable byte buffer and ring buffer implementation.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_buffer.h"

#include <stdint.h>
#include <string.h>

static bool cl_buffer_allocation_size(size_t capacity, size_t *out)
{
    if (!out) {
        return false;
    }

    *out = capacity;
    return true;
}

static bool cl_buffer_next_capacity(
    size_t current_capacity,
    size_t min_capacity,
    size_t *out_capacity)
{
    size_t next;

    if (!out_capacity) {
        return false;
    }

    if (min_capacity <= current_capacity) {
        *out_capacity = current_capacity;
        return true;
    }

    next = current_capacity == 0u ? 64u : current_capacity;
    while (next < min_capacity) {
        if (next > SIZE_MAX / 2u) {
            next = min_capacity;
            break;
        }
        next *= 2u;
    }

    *out_capacity = next;
    return true;
}

static size_t cl_ring_buffer_tail_index(const cl_ring_buffer *ring)
{
    size_t tail;

    if (!ring || ring->capacity == 0u) {
        return 0u;
    }

    if (ring->size >= ring->capacity - ring->head) {
        tail = ring->size - (ring->capacity - ring->head);
    } else {
        tail = ring->head + ring->size;
    }
    return tail;
}

static size_t cl_min_size(size_t a, size_t b)
{
    return a < b ? a : b;
}

void cl_buffer_init(cl_buffer *buffer, cl_allocator *allocator)
{
    if (!buffer) {
        return;
    }

    buffer->data = NULL;
    buffer->size = 0u;
    buffer->capacity = 0u;
    buffer->allocator = allocator;
}

void cl_buffer_clear(cl_buffer *buffer)
{
    if (!buffer) {
        return;
    }

    buffer->size = 0u;
}

void cl_buffer_free(cl_buffer *buffer)
{
    if (!buffer) {
        return;
    }

    cl_free(buffer->allocator, buffer->data, buffer->capacity, 1u);
    buffer->data = NULL;
    buffer->size = 0u;
    buffer->capacity = 0u;
}

bool cl_buffer_reserve(cl_buffer *buffer, size_t min_capacity)
{
    size_t old_size;
    size_t new_size;
    size_t next_capacity;
    void *next;

    if (!buffer) {
        return false;
    }

    if (min_capacity <= buffer->capacity) {
        return true;
    }

    if (!cl_buffer_next_capacity(
            buffer->capacity, min_capacity, &next_capacity) ||
        !cl_buffer_allocation_size(buffer->capacity, &old_size) ||
        !cl_buffer_allocation_size(next_capacity, &new_size)) {
        return false;
    }

    if (buffer->data) {
        next = cl_resize(buffer->allocator, buffer->data, old_size, new_size, 1u);
    } else {
        next = cl_alloc(buffer->allocator, new_size, 1u);
    }

    if (!next) {
        return false;
    }

    buffer->data = (unsigned char *)next;
    buffer->capacity = next_capacity;
    return true;
}

bool cl_buffer_resize(cl_buffer *buffer, size_t new_size)
{
    if (!buffer) {
        return false;
    }

    if (new_size > buffer->capacity && !cl_buffer_reserve(buffer, new_size)) {
        return false;
    }

    if (new_size > buffer->size) {
        memset(buffer->data + buffer->size, 0, new_size - buffer->size);
    }
    buffer->size = new_size;
    return true;
}

bool cl_buffer_append(cl_buffer *buffer, const void *data, size_t size)
{
    if (!buffer || (size != 0u && !data)) {
        return false;
    }

    if (size > SIZE_MAX - buffer->size) {
        return false;
    }

    if (!cl_buffer_reserve(buffer, buffer->size + size)) {
        return false;
    }

    if (size != 0u) {
        memcpy(buffer->data + buffer->size, data, size);
        buffer->size += size;
    }
    return true;
}

bool cl_buffer_append_byte(cl_buffer *buffer, unsigned char byte)
{
    return cl_buffer_append(buffer, &byte, 1u);
}

void cl_ring_buffer_init(cl_ring_buffer *ring, void *storage, size_t capacity)
{
    if (!ring) {
        return;
    }

    ring->data = (unsigned char *)storage;
    ring->capacity = storage ? capacity : 0u;
    ring->head = 0u;
    ring->size = 0u;
}

void cl_ring_buffer_reset(cl_ring_buffer *ring)
{
    if (!ring) {
        return;
    }

    ring->head = 0u;
    ring->size = 0u;
}

size_t cl_ring_buffer_size(const cl_ring_buffer *ring)
{
    return ring ? ring->size : 0u;
}

size_t cl_ring_buffer_capacity(const cl_ring_buffer *ring)
{
    return ring ? ring->capacity : 0u;
}

size_t cl_ring_buffer_available(const cl_ring_buffer *ring)
{
    if (!ring || ring->size > ring->capacity) {
        return 0u;
    }

    return ring->capacity - ring->size;
}

bool cl_ring_buffer_push(cl_ring_buffer *ring, unsigned char byte)
{
    size_t index;

    if (!ring || !ring->data || ring->size == ring->capacity) {
        return false;
    }

    index = cl_ring_buffer_tail_index(ring);
    ring->data[index] = byte;
    ++ring->size;
    return true;
}

bool cl_ring_buffer_pop(cl_ring_buffer *ring, unsigned char *out)
{
    if (!ring || !ring->data || ring->size == 0u) {
        return false;
    }

    if (out) {
        *out = ring->data[ring->head];
    }

    ++ring->head;
    if (ring->head == ring->capacity) {
        ring->head = 0u;
    }
    --ring->size;
    return true;
}

size_t cl_ring_buffer_write(cl_ring_buffer *ring, const void *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t available;
    size_t first;
    size_t second;
    size_t tail;
    size_t written;

    if (!ring || (size != 0u && !bytes)) {
        return 0u;
    }

    available = cl_ring_buffer_available(ring);
    written = cl_min_size(size, available);
    if (written == 0u || !ring->data || ring->capacity == 0u) {
        return 0u;
    }

    tail = cl_ring_buffer_tail_index(ring);
    first = cl_min_size(written, ring->capacity - tail);
    memcpy(ring->data + tail, bytes, first);
    second = written - first;
    if (second != 0u) {
        memcpy(ring->data, bytes + first, second);
    }

    ring->size += written;
    return written;
}

size_t cl_ring_buffer_read(cl_ring_buffer *ring, void *out, size_t size)
{
    unsigned char *bytes = (unsigned char *)out;
    size_t first;
    size_t read;
    size_t second;

    if (!ring || (size != 0u && !bytes)) {
        return 0u;
    }

    read = cl_min_size(size, ring->size);
    if (read == 0u || !ring->data || ring->capacity == 0u) {
        return 0u;
    }

    first = cl_min_size(read, ring->capacity - ring->head);
    memcpy(bytes, ring->data + ring->head, first);
    second = read - first;
    if (second != 0u) {
        memcpy(bytes + first, ring->data, second);
    }

    ring->head += first;
    if (ring->head == ring->capacity) {
        ring->head = 0u;
    }
    if (second != 0u) {
        ring->head = second;
    }
    ring->size -= read;
    return read;
}
