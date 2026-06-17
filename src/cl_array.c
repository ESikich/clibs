/*
 * cl_array.c
 * Purpose: Shared checked allocation helpers for typed dynamic arrays.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_array.h"

#include <stdint.h>

bool cl_array_allocation_size(size_t capacity, size_t elem_size, size_t *out)
{
    if (!out || elem_size == 0u) {
        return false;
    }

    if (capacity != 0u && elem_size > SIZE_MAX / capacity) {
        return false;
    }

    *out = capacity * elem_size;
    return true;
}

bool cl_array_next_capacity(
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

    next = current_capacity == 0u ? 8u : current_capacity;
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

bool cl_array_reallocate(
    cl_allocator *allocator,
    void **data,
    size_t old_capacity,
    size_t new_capacity,
    size_t elem_size,
    size_t elem_align)
{
    size_t old_size;
    size_t new_size;
    void *next;

    if (!data) {
        return false;
    }

    if (!cl_array_allocation_size(old_capacity, elem_size, &old_size) ||
        !cl_array_allocation_size(new_capacity, elem_size, &new_size)) {
        return false;
    }

    if (!*data && old_capacity != 0u) {
        return false;
    }

    if (new_capacity == 0u) {
        cl_free(allocator, *data, old_size, elem_align);
        *data = NULL;
        return true;
    }

    if (*data) {
        next = cl_resize(allocator, *data, old_size, new_size, elem_align);
    } else {
        next = cl_alloc(allocator, new_size, elem_align);
    }

    if (!next) {
        return false;
    }

    *data = next;
    return true;
}
