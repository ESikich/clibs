/*
 * cl_bitset.h
 * Purpose: Fixed-storage and allocator-backed bitset helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_BITSET_H
#define CL_BITSET_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_bitset {
    size_t *words;
    size_t bit_count;
    size_t word_count;
    cl_allocator *allocator;
} cl_bitset;

bool cl_bitset_required_bytes(size_t bit_count, size_t *out_bytes);
bool cl_bitset_init(cl_bitset *set, cl_allocator *allocator, size_t bit_count);
bool cl_bitset_init_with_storage(
    cl_bitset *set,
    void *storage,
    size_t storage_size,
    size_t bit_count);
void cl_bitset_free(cl_bitset *set);

size_t cl_bitset_size(const cl_bitset *set);
bool cl_bitset_get(const cl_bitset *set, size_t index, bool *out);
bool cl_bitset_set(cl_bitset *set, size_t index);
bool cl_bitset_clear(cl_bitset *set, size_t index);
bool cl_bitset_toggle(cl_bitset *set, size_t index);
void cl_bitset_clear_all(cl_bitset *set);
void cl_bitset_fill_all(cl_bitset *set);

size_t cl_bitset_count(const cl_bitset *set);
bool cl_bitset_find_first_set(
    const cl_bitset *set,
    size_t start,
    size_t *out_index);
bool cl_bitset_find_first_clear(
    const cl_bitset *set,
    size_t start,
    size_t *out_index);

bool cl_bitset_and(cl_bitset *dst, const cl_bitset *rhs);
bool cl_bitset_or(cl_bitset *dst, const cl_bitset *rhs);
bool cl_bitset_xor(cl_bitset *dst, const cl_bitset *rhs);
void cl_bitset_not(cl_bitset *set);

#ifdef __cplusplus
}
#endif

#endif
