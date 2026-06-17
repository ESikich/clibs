<!--
docs/cl_alloc.md
Purpose: Allocator library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_alloc

`cl_alloc` is the first foundation library in this repository. It provides a
small explicit allocator interface and three allocator implementations.

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
- `cl_debug_allocator` wraps another allocator and adds guard bytes, mismatch
  detection, double-free detection for its own pointers, and live/peak counters.

## Safety Properties

- Zero-size allocations return `NULL`.
- Alignment must be a non-zero power of two.
- Internal size additions and alignment rounding are checked for overflow.
- Arena restore only accepts marks at or before the current arena offset.
- Debug allocations are quarantined until `cl_debug_allocator_release` so double
  frees can be detected without reading memory already returned to the backing
  allocator.

## Portability

The implementation targets POSIX.1-2008 compatible C99. `posix_memalign` is used
for over-aligned system allocations and is exposed through
`_POSIX_C_SOURCE=200809L` in the default build.
