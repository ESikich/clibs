<!--
docs/cl_map.md
Purpose: Ordered map library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_map

`cl_map` provides an allocator-backed ordered map for non-owning byte keys and
caller-owned pointer values.

## Example

```c
cl_allocator allocator = cl_system_allocator();
cl_map map;
int count = 4;
void *found = NULL;

cl_map_init(&map, &allocator);

if (cl_map_put_cstr(&map, "apples", &count) &&
    cl_map_get_cstr(&map, "apples", &found)) {
    *(int *)found += 1;
}

cl_map_free(&map);
```

## Contracts

- The allocator passed to `init` must outlive the map.
- The map owns only its tree nodes.
- Keys are not copied. Key memory must remain valid and unchanged while an entry
  references it.
- Values are stored as `void *`; pointed-to objects remain caller-owned.
- Byte-key APIs accept embedded zero bytes through explicit `key_size`.
- C-string helpers use `strlen` and reject `NULL` keys.
- Iterators are invalidated by insertions, removals, `clear`, and `free`.

## Operations

- `cl_map_put` inserts or updates a key.
- `cl_map_get` retrieves a value and returns `false` when absent.
- `cl_map_contains` tests for a key without copying the value.
- `cl_map_remove` removes a key and optionally returns its value.
- `cl_map_first` and `cl_map_last` start ordered iteration.
- `cl_map_next` and `cl_map_prev` move between entries in byte-lexicographic
  order.
- `cl_map_clear` drops all entries.
- `cl_map_free` releases tree nodes and resets the map.

## Safety Properties

- Each node allocation uses explicit size and alignment.
- Failed insert allocation leaves the original map unchanged.
- Removal returns the caller-owned value before unlinking and freeing the node.
- AVL rebalancing keeps lookup, insertion, removal, and iteration steps bounded
  by tree height.

## Portability

The implementation targets POSIX.1-2008 compatible C99. It uses only standard C
byte comparisons and allocator calls.
