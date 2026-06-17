/*
 * cl_hash.h
 * Purpose: Hash functions and non-owning key hash table for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_HASH_H
#define CL_HASH_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t cl_hash_fnv1a64(const void *data, size_t size);
uint64_t cl_hash_cstr(const char *text);

typedef struct cl_hash_table {
    void *entries;
    size_t size;
    size_t capacity;
    size_t tombstones;
    cl_allocator *allocator;
} cl_hash_table;

/*
 * Hash tables own only their bucket storage. Keys and pointed-to values remain
 * caller-owned and must outlive entries that reference them.
 */
void cl_hash_table_init(cl_hash_table *table, cl_allocator *allocator);
void cl_hash_table_clear(cl_hash_table *table);
void cl_hash_table_free(cl_hash_table *table);
size_t cl_hash_table_size(const cl_hash_table *table);
size_t cl_hash_table_capacity(const cl_hash_table *table);
bool cl_hash_table_reserve(cl_hash_table *table, size_t min_capacity);
bool cl_hash_table_put(
    cl_hash_table *table,
    const void *key,
    size_t key_size,
    void *value);
bool cl_hash_table_get(
    const cl_hash_table *table,
    const void *key,
    size_t key_size,
    void **out_value);
bool cl_hash_table_contains(
    const cl_hash_table *table,
    const void *key,
    size_t key_size);
bool cl_hash_table_remove(
    cl_hash_table *table,
    const void *key,
    size_t key_size,
    void **out_value);

bool cl_hash_table_put_cstr(cl_hash_table *table, const char *key, void *value);
bool cl_hash_table_get_cstr(
    const cl_hash_table *table,
    const char *key,
    void **out_value);
bool cl_hash_table_contains_cstr(const cl_hash_table *table, const char *key);
bool cl_hash_table_remove_cstr(
    cl_hash_table *table,
    const char *key,
    void **out_value);

#ifdef __cplusplus
}
#endif

#endif
