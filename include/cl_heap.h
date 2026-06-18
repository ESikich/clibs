/*
 * cl_heap.h
 * Purpose: Allocation-free binary heap algorithms for caller-owned arrays.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_HEAP_H
#define CL_HEAP_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cl_heap_compare)(
    const void *left,
    const void *right,
    void *user);

bool cl_heap_make(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user);
bool cl_heap_push(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user);
bool cl_heap_pop(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user);
bool cl_heap_sort(
    void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user);
bool cl_heap_is_valid(
    const void *base,
    size_t count,
    size_t element_size,
    cl_heap_compare compare,
    void *user);

#ifdef __cplusplus
}
#endif

#endif
