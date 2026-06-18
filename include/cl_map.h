/*
 * cl_map.h
 * Purpose: Ordered non-owning byte-key map for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_MAP_H
#define CL_MAP_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_map {
    void *root;
    size_t size;
    cl_allocator *allocator;
} cl_map;

typedef struct cl_map_iter {
    void *node;
} cl_map_iter;

/*
 * Maps own only their tree nodes. Keys and pointed-to values remain
 * caller-owned and must outlive entries that reference them.
 */
void cl_map_init(cl_map *map, cl_allocator *allocator);
void cl_map_clear(cl_map *map);
void cl_map_free(cl_map *map);
size_t cl_map_size(const cl_map *map);
bool cl_map_is_empty(const cl_map *map);

bool cl_map_put(cl_map *map, const void *key, size_t key_size, void *value);
bool cl_map_get(
    const cl_map *map,
    const void *key,
    size_t key_size,
    void **out_value);
bool cl_map_contains(const cl_map *map, const void *key, size_t key_size);
bool cl_map_remove(
    cl_map *map,
    const void *key,
    size_t key_size,
    void **out_value);

bool cl_map_put_cstr(cl_map *map, const char *key, void *value);
bool cl_map_get_cstr(const cl_map *map, const char *key, void **out_value);
bool cl_map_contains_cstr(const cl_map *map, const char *key);
bool cl_map_remove_cstr(cl_map *map, const char *key, void **out_value);

cl_map_iter cl_map_first(const cl_map *map);
cl_map_iter cl_map_last(const cl_map *map);
cl_map_iter cl_map_next(cl_map_iter iter);
cl_map_iter cl_map_prev(cl_map_iter iter);
bool cl_map_iter_is_valid(cl_map_iter iter);
const void *cl_map_iter_key(cl_map_iter iter, size_t *out_key_size);
void *cl_map_iter_value(cl_map_iter iter);

#ifdef __cplusplus
}
#endif

#endif
