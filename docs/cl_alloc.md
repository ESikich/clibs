<!--
docs/cl_alloc.md
Purpose: Allocator library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_alloc

`cl_alloc` is the first foundation library in this repository. It provides a
small explicit allocator interface and five allocator implementations.

## Interface

The allocator interface passes size and alignment through allocation, resize, and
free operations:

```c
void *cl_alloc(cl_allocator *allocator, size_t size, size_t align);
void *cl_resize(
    cl_allocator *allocator,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align);
void cl_free(cl_allocator *allocator, void *ptr, size_t size, size_t align);
```

Callers are responsible for passing the same size and alignment to `cl_free`
that were used for the allocation. The debug allocator uses that contract to
detect mismatches.

## Implementations

- `cl_system_allocator` wraps `malloc`, `realloc`, `free`, and `posix_memalign`.
- `cl_arena_allocator` provides bump allocation over caller-owned memory.
- `cl_pool_allocator` provides fixed-size block allocation over caller-owned
  memory.
- `cl_free_list_allocator` provides variable-size allocation over caller-owned
  memory, with block splitting and coalescing.
- `cl_debug_allocator` wraps another allocator and adds guard bytes, mismatch
  detection, double-free detection for its own pointers, and live/peak counters.

## Fixed-Size Pools

Pools are initialized with caller-owned storage plus a block size and block
alignment:

```c
unsigned char storage[4096];
cl_pool pool;
cl_allocator allocator;

if (cl_pool_init(&pool, storage, sizeof(storage), 64, 16)) {
    allocator = cl_pool_allocator(&pool);
}
```

Each successful allocation consumes one block. Requests must be non-zero, no
larger than the configured block size, and no more strictly aligned than the
configured block alignment. `cl_pool_reset` rebuilds the free list and
invalidates outstanding allocations. Pools reserve one byte of caller storage
per slot for allocation-state diagnostics, so `cl_pool_block_count` is the
authoritative usable block count after initialization. Freed slot contents are
allocator-owned metadata until the slot is allocated again.

Pool frees and resizes record contract violations through
`cl_pool_invalid_free_count`, `cl_pool_mismatch_count`, and
`cl_pool_double_free_count`. Invalid frees include foreign pointers and pointers
inside a slot; mismatches include wrong sizes or alignments; double frees are
recognized before the slot is linked into the free list again.

## Free Lists

Free lists are initialized with caller-owned storage:

```c
unsigned char storage[4096];
cl_free_list list;
cl_allocator allocator;

if (cl_free_list_init(&list, storage, sizeof(storage))) {
    allocator = cl_free_list_allocator(&list);
}
```

Each allocation stores a small header immediately before the returned pointer.
Small freed blocks are cached in size-class bins for fast reuse. Larger freed
blocks are inserted in address order and coalesced with adjacent free blocks;
cached small blocks are flushed back to the coalescing list when larger
allocations need contiguous storage. Requests must be non-zero and use a
power-of-two alignment. `cl_free` must receive the same size and alignment used
for the allocation; mismatches are rejected and leave the allocation owned by
the caller. `cl_free_list_reset` rebuilds one free block over the full backing
storage and invalidates outstanding allocations.

Free-list frees and resizes record contract violations through
`cl_free_list_invalid_free_count`, `cl_free_list_mismatch_count`, and
`cl_free_list_double_free_count`.

## Safety Properties

- Zero-size allocations return `NULL`.
- Alignment must be a non-zero power of two.
- Internal size additions and alignment rounding are checked for overflow.
- Arena restore only accepts marks at or before the current arena offset.
- Pool frees reject pointers outside the pool and pointers that do not point to
  the start of a pool slot. Pool double frees are rejected before they can link
  the same slot into the free list twice.
- Free-list frees reject pointers outside the managed storage and size/alignment
  mismatches for recognized free-list allocations. Free-list double frees are
  detected when the pointer falls inside an already-free block.
- Debug allocations are quarantined until `cl_debug_allocator_release` so double
  frees can be detected without reading memory already returned to the backing
  allocator.

## Portability

The implementation targets POSIX.1-2008 compatible C99. `posix_memalign` is used
for over-aligned system allocations and is exposed through
`_POSIX_C_SOURCE=200809L` in the default build.
