/*
 * cl_set.c
 * Purpose: Implement non-owning byte-key hash sets.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_set.h"

void cl_set_init(cl_set *set, cl_allocator *allocator)
{
    if (!set) {
        return;
    }

    cl_hash_table_init(&set->table, allocator);
}

void cl_set_clear(cl_set *set)
{
    if (!set) {
        return;
    }

    cl_hash_table_clear(&set->table);
}

void cl_set_free(cl_set *set)
{
    if (!set) {
        return;
    }

    cl_hash_table_free(&set->table);
}

size_t cl_set_size(const cl_set *set)
{
    return set ? cl_hash_table_size(&set->table) : 0u;
}

size_t cl_set_capacity(const cl_set *set)
{
    return set ? cl_hash_table_capacity(&set->table) : 0u;
}

bool cl_set_reserve(cl_set *set, size_t min_capacity)
{
    return set ? cl_hash_table_reserve(&set->table, min_capacity) : false;
}

bool cl_set_insert(cl_set *set, const void *key, size_t key_size)
{
    return set ? cl_hash_table_put(&set->table, key, key_size, NULL) : false;
}

bool cl_set_contains(const cl_set *set, const void *key, size_t key_size)
{
    return set ? cl_hash_table_contains(&set->table, key, key_size) : false;
}

bool cl_set_remove(cl_set *set, const void *key, size_t key_size)
{
    return set ? cl_hash_table_remove(&set->table, key, key_size, NULL) : false;
}

bool cl_set_insert_cstr(cl_set *set, const char *key)
{
    return set ? cl_hash_table_put_cstr(&set->table, key, NULL) : false;
}

bool cl_set_contains_cstr(const cl_set *set, const char *key)
{
    return set ? cl_hash_table_contains_cstr(&set->table, key) : false;
}

bool cl_set_remove_cstr(cl_set *set, const char *key)
{
    return set ? cl_hash_table_remove_cstr(&set->table, key, NULL) : false;
}
