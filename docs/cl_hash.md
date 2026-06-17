<!--
docs/cl_hash.md
Purpose: Hash function and hash table library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_hash

`cl_hash` provides FNV-1a 64-bit hashing plus an allocator-backed hash table for
non-owning byte keys and caller-owned pointer values.

## Example

```c
cl_allocator allocator = cl_system_allocator();
cl_hash_table table;
int count = 4;
void *found = NULL;

cl_hash_table_init(&table, &allocator);

if (cl_hash_table_put_cstr(&table, "apples", &count) &&
    cl_hash_table_get_cstr(&table, "apples", &found)) {
    *(int *)found += 1;
}

cl_hash_table_free(&table);
```

## Contracts

- The allocator passed to `init` must outlive the table.
- The table owns only its bucket array.
- Keys are not copied. Key memory must remain valid and unchanged while an entry
  references it.
- Values are stored as `void *`; pointed-to objects remain caller-owned.
- Byte-key APIs accept embedded zero bytes through explicit `key_size`.
- C-string helpers use `strlen` and reject `NULL` keys.

## Operations

- `cl_hash_fnv1a64` hashes explicit byte ranges.
- `cl_hash_cstr` hashes null-terminated strings.
- `cl_hash_table_put` inserts or updates a key.
- `cl_hash_table_get` retrieves a value and returns `false` when absent.
- `cl_hash_table_contains` tests for a key without copying the value.
- `cl_hash_table_remove` removes a key and optionally returns its value.
- `cl_hash_table_clear` drops all entries but keeps capacity.
- `cl_hash_table_free` releases buckets and resets the table.

## Safety Properties

- Capacity and allocation byte counts use checked arithmetic before allocation.
- Failed growth leaves the original table intact.
- Deletion uses tombstones so later lookups across the same probe chain remain
  valid.
- Rehashing removes tombstones and preserves all live entries before freeing the
  old bucket array.

## Portability

The implementation targets POSIX.1-2008 compatible C99. It uses linear probing,
power-of-two capacities, and only standard C library byte operations.
