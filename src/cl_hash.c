/*
 * cl_hash.c
 * Purpose: Hash functions and open-addressed hash table implementation.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_hash.h"

#include <stdint.h>
#include <string.h>

#define CL_HASH_FNV_OFFSET 14695981039346656037ull
#define CL_HASH_FNV_PRIME 1099511628211ull
#define CL_HASH_MIN_CAPACITY 16u

typedef enum cl_hash_entry_state {
    CL_HASH_ENTRY_EMPTY = 0,
    CL_HASH_ENTRY_OCCUPIED = 1,
    CL_HASH_ENTRY_TOMBSTONE = 2
} cl_hash_entry_state;

typedef struct cl_hash_entry {
    uint64_t hash;
    const void *key;
    size_t key_size;
    void *value;
    unsigned char state;
} cl_hash_entry;

static size_t cl_hash_entry_align(void)
{
    typedef struct cl_hash_entry_align_probe {
        char c;
        cl_hash_entry entry;
    } cl_hash_entry_align_probe;

    return offsetof(cl_hash_entry_align_probe, entry);
}

static bool cl_hash_allocation_size(size_t capacity, size_t *out)
{
    if (!out) {
        return false;
    }

    if (capacity != 0u && sizeof(cl_hash_entry) > SIZE_MAX / capacity) {
        return false;
    }

    *out = capacity * sizeof(cl_hash_entry);
    return true;
}

static bool cl_hash_next_capacity(size_t min_capacity, size_t *out_capacity)
{
    size_t next;

    if (!out_capacity) {
        return false;
    }

    next = CL_HASH_MIN_CAPACITY;
    while (next < min_capacity) {
        if (next > SIZE_MAX / 2u) {
            return false;
        }
        next *= 2u;
    }

    *out_capacity = next;
    return true;
}

static bool cl_hash_min_capacity_for_size(size_t size, size_t *out_capacity)
{
    size_t capacity;

    if (!cl_hash_next_capacity(size, &capacity)) {
        return false;
    }

    while (size > capacity - (capacity / 4u)) {
        if (capacity > SIZE_MAX / 2u) {
            return false;
        }
        capacity *= 2u;
    }

    *out_capacity = capacity;
    return true;
}

static bool cl_hash_key_equal(
    const cl_hash_entry *entry,
    uint64_t hash,
    const void *key,
    size_t key_size)
{
    if (entry->state != (unsigned char)CL_HASH_ENTRY_OCCUPIED ||
        entry->hash != hash || entry->key_size != key_size) {
        return false;
    }

    return key_size == 0u || memcmp(entry->key, key, key_size) == 0;
}

static bool cl_hash_should_grow(const cl_hash_table *table)
{
    size_t used_slots;

    if (!table || table->capacity == 0u) {
        return true;
    }

    if (table->size >= SIZE_MAX - table->tombstones) {
        return true;
    }

    used_slots = table->size + table->tombstones + 1u;
    return used_slots > table->capacity - (table->capacity / 4u);
}

static bool cl_hash_find_slot(
    const cl_hash_entry *entries,
    size_t capacity,
    uint64_t hash,
    const void *key,
    size_t key_size,
    size_t *out_index,
    bool *out_found)
{
    size_t first_tombstone = (size_t)-1;
    size_t index;
    size_t i;

    if (!entries || capacity == 0u || !out_index || !out_found) {
        return false;
    }

    index = (size_t)hash & (capacity - 1u);
    for (i = 0u; i < capacity; ++i) {
        const cl_hash_entry *entry = &entries[index];

        if (entry->state == (unsigned char)CL_HASH_ENTRY_EMPTY) {
            *out_index = first_tombstone != (size_t)-1 ? first_tombstone : index;
            *out_found = false;
            return true;
        }

        if (entry->state == (unsigned char)CL_HASH_ENTRY_TOMBSTONE) {
            if (first_tombstone == (size_t)-1) {
                first_tombstone = index;
            }
        } else if (cl_hash_key_equal(entry, hash, key, key_size)) {
            *out_index = index;
            *out_found = true;
            return true;
        }

        index = (index + 1u) & (capacity - 1u);
    }

    if (first_tombstone != (size_t)-1) {
        *out_index = first_tombstone;
        *out_found = false;
        return true;
    }

    return false;
}

static bool cl_hash_rehash(cl_hash_table *table, size_t min_capacity)
{
    cl_hash_entry *old_entries;
    cl_hash_entry *new_entries;
    size_t old_capacity;
    size_t new_capacity;
    size_t old_size;
    size_t new_size;
    size_t i;

    if (table && min_capacity < table->size &&
        !cl_hash_min_capacity_for_size(table->size, &min_capacity)) {
        return false;
    }

    if (!table || !cl_hash_next_capacity(min_capacity, &new_capacity) ||
        !cl_hash_allocation_size(new_capacity, &new_size)) {
        return false;
    }

    while (table->size > new_capacity - (new_capacity / 4u)) {
        if (new_capacity > SIZE_MAX / 2u) {
            return false;
        }
        new_capacity *= 2u;
        if (!cl_hash_allocation_size(new_capacity, &new_size)) {
            return false;
        }
    }

    new_entries = cl_alloc(table->allocator, new_size, cl_hash_entry_align());
    if (!new_entries) {
        return false;
    }
    memset(new_entries, 0, new_size);

    old_entries = (cl_hash_entry *)table->entries;
    old_capacity = table->capacity;
    for (i = 0u; i < old_capacity; ++i) {
        size_t index;
        bool found;

        if (old_entries[i].state != (unsigned char)CL_HASH_ENTRY_OCCUPIED) {
            continue;
        }

        if (!cl_hash_find_slot(
                new_entries,
                new_capacity,
                old_entries[i].hash,
                old_entries[i].key,
                old_entries[i].key_size,
                &index,
                &found)) {
            cl_free(table->allocator, new_entries, new_size, cl_hash_entry_align());
            return false;
        }
        new_entries[index] = old_entries[i];
    }

    if (old_entries) {
        if (!cl_hash_allocation_size(old_capacity, &old_size)) {
            cl_free(table->allocator, new_entries, new_size, cl_hash_entry_align());
            return false;
        }
        cl_free(table->allocator, old_entries, old_size, cl_hash_entry_align());
    }

    table->entries = new_entries;
    table->capacity = new_capacity;
    table->tombstones = 0u;
    return true;
}

uint64_t cl_hash_fnv1a64(const void *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    uint64_t hash = CL_HASH_FNV_OFFSET;
    size_t i;

    if (size != 0u && !bytes) {
        return 0u;
    }

    for (i = 0u; i < size; ++i) {
        hash ^= (uint64_t)bytes[i];
        hash *= CL_HASH_FNV_PRIME;
    }

    return hash;
}

uint64_t cl_hash_cstr(const char *text)
{
    if (!text) {
        return 0u;
    }

    return cl_hash_fnv1a64(text, strlen(text));
}

void cl_hash_table_init(cl_hash_table *table, cl_allocator *allocator)
{
    if (!table) {
        return;
    }

    table->entries = NULL;
    table->size = 0u;
    table->capacity = 0u;
    table->tombstones = 0u;
    table->allocator = allocator;
}

void cl_hash_table_clear(cl_hash_table *table)
{
    size_t bytes;

    if (!table || !table->entries ||
        !cl_hash_allocation_size(table->capacity, &bytes)) {
        return;
    }

    memset(table->entries, 0, bytes);
    table->size = 0u;
    table->tombstones = 0u;
}

void cl_hash_table_free(cl_hash_table *table)
{
    size_t bytes;

    if (!table) {
        return;
    }

    if (table->entries && cl_hash_allocation_size(table->capacity, &bytes)) {
        cl_free(table->allocator, table->entries, bytes, cl_hash_entry_align());
    }

    table->entries = NULL;
    table->size = 0u;
    table->capacity = 0u;
    table->tombstones = 0u;
}

size_t cl_hash_table_size(const cl_hash_table *table)
{
    return table ? table->size : 0u;
}

size_t cl_hash_table_capacity(const cl_hash_table *table)
{
    return table ? table->capacity : 0u;
}

bool cl_hash_table_reserve(cl_hash_table *table, size_t min_capacity)
{
    if (!table) {
        return false;
    }

    if (min_capacity <= table->capacity && table->tombstones == 0u) {
        return true;
    }

    if (min_capacity <= table->capacity) {
        min_capacity = table->capacity;
    }

    return cl_hash_rehash(table, min_capacity);
}

bool cl_hash_table_put(
    cl_hash_table *table,
    const void *key,
    size_t key_size,
    void *value)
{
    cl_hash_entry *entries;
    uint64_t hash;
    size_t index;
    bool found;

    if (!table || (key_size != 0u && !key)) {
        return false;
    }

    if (cl_hash_should_grow(table)) {
        size_t next_capacity = CL_HASH_MIN_CAPACITY;

        if (table->capacity != 0u) {
            if (table->capacity > SIZE_MAX / 2u) {
                return false;
            }
            next_capacity = table->capacity * 2u;
        }

        if (!cl_hash_rehash(table, next_capacity)) {
            return false;
        }
    }

    entries = (cl_hash_entry *)table->entries;
    hash = cl_hash_fnv1a64(key, key_size);
    if (!cl_hash_find_slot(
            entries, table->capacity, hash, key, key_size, &index, &found)) {
        return false;
    }

    if (found) {
        entries[index].value = value;
        return true;
    }

    if (entries[index].state == (unsigned char)CL_HASH_ENTRY_TOMBSTONE) {
        --table->tombstones;
    }

    entries[index].hash = hash;
    entries[index].key = key;
    entries[index].key_size = key_size;
    entries[index].value = value;
    entries[index].state = (unsigned char)CL_HASH_ENTRY_OCCUPIED;
    ++table->size;
    return true;
}

bool cl_hash_table_get(
    const cl_hash_table *table,
    const void *key,
    size_t key_size,
    void **out_value)
{
    size_t index;
    bool found;
    uint64_t hash;

    if (!table || !table->entries || (key_size != 0u && !key)) {
        return false;
    }

    hash = cl_hash_fnv1a64(key, key_size);
    if (!cl_hash_find_slot(
            (const cl_hash_entry *)table->entries,
            table->capacity,
            hash,
            key,
            key_size,
            &index,
            &found) ||
        !found) {
        return false;
    }

    if (out_value) {
        *out_value = ((cl_hash_entry *)table->entries)[index].value;
    }
    return true;
}

bool cl_hash_table_contains(
    const cl_hash_table *table,
    const void *key,
    size_t key_size)
{
    return cl_hash_table_get(table, key, key_size, NULL);
}

bool cl_hash_table_remove(
    cl_hash_table *table,
    const void *key,
    size_t key_size,
    void **out_value)
{
    cl_hash_entry *entries;
    size_t index;
    bool found;
    uint64_t hash;

    if (!table || !table->entries || (key_size != 0u && !key)) {
        return false;
    }

    entries = (cl_hash_entry *)table->entries;
    hash = cl_hash_fnv1a64(key, key_size);
    if (!cl_hash_find_slot(
            entries, table->capacity, hash, key, key_size, &index, &found) ||
        !found) {
        return false;
    }

    if (out_value) {
        *out_value = entries[index].value;
    }

    entries[index].key = NULL;
    entries[index].key_size = 0u;
    entries[index].value = NULL;
    entries[index].state = (unsigned char)CL_HASH_ENTRY_TOMBSTONE;
    --table->size;
    ++table->tombstones;
    return true;
}

bool cl_hash_table_put_cstr(cl_hash_table *table, const char *key, void *value)
{
    if (!key) {
        return false;
    }

    return cl_hash_table_put(table, key, strlen(key), value);
}

bool cl_hash_table_get_cstr(
    const cl_hash_table *table,
    const char *key,
    void **out_value)
{
    if (!key) {
        return false;
    }

    return cl_hash_table_get(table, key, strlen(key), out_value);
}

bool cl_hash_table_contains_cstr(const cl_hash_table *table, const char *key)
{
    return cl_hash_table_get_cstr(table, key, NULL);
}

bool cl_hash_table_remove_cstr(
    cl_hash_table *table,
    const char *key,
    void **out_value)
{
    if (!key) {
        return false;
    }

    return cl_hash_table_remove(table, key, strlen(key), out_value);
}
