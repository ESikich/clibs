/*
 * cl_set.h
 * Purpose: Non-owning byte-key hash set for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_SET_H
#define CL_SET_H

#include "cl_hash.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_set {
    cl_hash_table table;
} cl_set;

/*
 * Sets own only their bucket storage. Keys remain caller-owned and must outlive
 * entries that reference them.
 */
void cl_set_init(cl_set *set, cl_allocator *allocator);
void cl_set_clear(cl_set *set);
void cl_set_free(cl_set *set);
size_t cl_set_size(const cl_set *set);
size_t cl_set_capacity(const cl_set *set);
bool cl_set_reserve(cl_set *set, size_t min_capacity);
bool cl_set_insert(cl_set *set, const void *key, size_t key_size);
bool cl_set_contains(const cl_set *set, const void *key, size_t key_size);
bool cl_set_remove(cl_set *set, const void *key, size_t key_size);

bool cl_set_insert_cstr(cl_set *set, const char *key);
bool cl_set_contains_cstr(const cl_set *set, const char *key);
bool cl_set_remove_cstr(cl_set *set, const char *key);

#ifdef __cplusplus
}
#endif

#endif
