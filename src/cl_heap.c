/*
 * cl_heap.c
 * Purpose: Implement allocation-free binary heap algorithms.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_heap.h"

#include <string.h>

static bool cl_heap_params_valid(
    const void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare)
{
    if (element_size == 0u || !compare) {
        return false;
    }
    if (count != 0u && !base) {
        return false;
    }
    if (count != 0u && element_size > ((size_t)-1) / count) {
        return false;
    }
    return true;
}

static unsigned char *cl_heap_slot(
    unsigned char *base,
    size_t index,
    size_t element_size)
{
    return base + (index * element_size);
}

static const unsigned char *cl_heap_const_slot(
    const unsigned char *base,
    size_t index,
    size_t element_size)
{
    return base + (index * element_size);
}

static bool cl_heap_higher(
    const unsigned char *base,
    size_t left,
    size_t right,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    return compare(
               cl_heap_const_slot(base, left, element_size),
               cl_heap_const_slot(base, right, element_size),
               user) > 0;
}

static void cl_heap_swap(
    unsigned char *base,
    size_t left,
    size_t right,
    size_t element_size)
{
    unsigned char *left_slot;
    unsigned char *right_slot;

    if (left == right) {
        return;
    }

    left_slot = cl_heap_slot(base, left, element_size);
    right_slot = cl_heap_slot(base, right, element_size);

    if (element_size == 8u) {
        unsigned char tmp[8];

        memcpy(tmp, left_slot, sizeof(tmp));
        memcpy(left_slot, right_slot, sizeof(tmp));
        memcpy(right_slot, tmp, sizeof(tmp));
        return;
    }
    if (element_size == 4u) {
        unsigned char tmp[4];

        memcpy(tmp, left_slot, sizeof(tmp));
        memcpy(left_slot, right_slot, sizeof(tmp));
        memcpy(right_slot, tmp, sizeof(tmp));
        return;
    }
    if (element_size == 2u) {
        unsigned char tmp[2];

        memcpy(tmp, left_slot, sizeof(tmp));
        memcpy(left_slot, right_slot, sizeof(tmp));
        memcpy(right_slot, tmp, sizeof(tmp));
        return;
    }

    {
        size_t i;

        for (i = 0u; i < element_size; ++i) {
            unsigned char tmp = left_slot[i];
            left_slot[i] = right_slot[i];
            right_slot[i] = tmp;
        }
    }
}

static void cl_heap_sift_up(
    unsigned char *base,
    size_t index,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    while (index > 0u) {
        size_t parent = (index - 1u) / 2u;

        if (!cl_heap_higher(base, index, parent, element_size, compare, user)) {
            return;
        }

        cl_heap_swap(base, index, parent, element_size);
        index = parent;
    }
}

static void cl_heap_sift_down(
    unsigned char *base,
    size_t count,
    size_t index,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    while (count >= 2u && index <= (count - 2u) / 2u) {
        size_t left = (index * 2u) + 1u;
        size_t right = left + 1u;
        size_t highest = index;

        if (cl_heap_higher(base, left, highest, element_size, compare, user)) {
            highest = left;
        }
        if (right < count &&
            cl_heap_higher(base, right, highest, element_size, compare, user)) {
            highest = right;
        }
        if (highest == index) {
            return;
        }

        cl_heap_swap(base, index, highest, element_size);
        index = highest;
    }
}

bool cl_heap_make(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t index;

    if (!cl_heap_params_valid(base, count, element_size, compare)) {
        return false;
    }
    if (count < 2u) {
        return true;
    }

    index = (count - 2u) / 2u;
    for (;;) {
        cl_heap_sift_down(bytes, count, index, element_size, compare, user);
        if (index == 0u) {
            break;
        }
        --index;
    }
    return true;
}

bool cl_heap_push(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    if (!cl_heap_params_valid(base, count, element_size, compare) ||
        count == 0u) {
        return false;
    }

    cl_heap_sift_up(
        (unsigned char *)base,
        count - 1u,
        element_size,
        compare,
        user);
    return true;
}

bool cl_heap_pop(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    unsigned char *bytes = (unsigned char *)base;

    if (!cl_heap_params_valid(base, count, element_size, compare) ||
        count == 0u) {
        return false;
    }

    cl_heap_swap(bytes, 0u, count - 1u, element_size);
    cl_heap_sift_down(bytes, count - 1u, 0u, element_size, compare, user);
    return true;
}

bool cl_heap_sort(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t heap_count;

    if (!cl_heap_make(base, count, element_size, compare, user)) {
        return false;
    }

    heap_count = count;
    while (heap_count > 1u) {
        cl_heap_swap(bytes, 0u, heap_count - 1u, element_size);
        --heap_count;
        cl_heap_sift_down(bytes, heap_count, 0u, element_size, compare, user);
    }
    return true;
}

bool cl_heap_is_valid(
    const void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user)
{
    const unsigned char *bytes = (const unsigned char *)base;
    size_t parent;

    if (!cl_heap_params_valid(base, count, element_size, compare)) {
        return false;
    }

    for (parent = 0u; parent < count; ++parent) {
        size_t left;
        size_t right;

        if (count < 2u || parent > (count - 2u) / 2u) {
            break;
        }

        left = (parent * 2u) + 1u;
        right = left + 1u;
        if (cl_heap_higher(bytes, left, parent, element_size, compare, user)) {
            return false;
        }
        if (right < count &&
            cl_heap_higher(bytes, right, parent, element_size, compare, user)) {
            return false;
        }
    }
    return true;
}
