<!--
docs/cl_set.md
Purpose: Hash set library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_set

`cl_set` provides an allocator-backed hash set for non-owning byte keys. It is a
small set-shaped wrapper around `cl_hash_table`, so it shares the same checked
growth, tombstone deletion, and key lifetime rules without requiring callers to
store placeholder values.

## Example

```c
cl_allocator allocator = cl_system_allocator();
cl_set seen;

cl_set_init(&seen, &allocator);

if (cl_set_insert_cstr(&seen, "apples") &&
    cl_set_contains_cstr(&seen, "apples")) {
    /* key was recorded */
}

cl_set_free(&seen);
```

## Contracts

- The allocator passed to `init` must outlive the set.
- The set owns only its bucket array.
- Keys are not copied. Key memory must remain valid and unchanged while an entry
  references it.
- Byte-key APIs accept embedded zero bytes through explicit `key_size`.
- C-string helpers use `strlen` through `cl_hash` and reject `NULL` keys.

## Operations

- `cl_set_insert` inserts a key. Inserting an existing key leaves the size
  unchanged.
- `cl_set_contains` tests for a key.
- `cl_set_remove` removes a key and returns `false` when absent.
- `cl_set_reserve` prepares bucket capacity and may also compact tombstones.
- `cl_set_clear` drops all entries but keeps capacity.
- `cl_set_free` releases buckets and resets the set.

## Safety Properties

- Capacity and allocation byte counts are checked by the underlying hash table.
- Failed growth leaves the original set intact.
- Deletion preserves probe chains so later lookups remain valid.
- The wrapper does not dereference stored values because set entries store no
  caller value.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and relies only on the
portable `cl_hash` implementation.
