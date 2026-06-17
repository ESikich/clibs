<!--
examples.md
Purpose: Example program guide for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# Examples

The overview example in `examples/overview.c` shows the current core libraries
working together in one small program:

- `cl_sv` trims, splits, and parses a comma-separated inventory string.
- `cl_array` stores parsed records in a typed dynamic array.
- `cl_alloc` supplies a free-list allocator wrapped by the debug allocator, plus
  arena scratch storage and a fixed-size pool allocation.
- `cl_libc` handles bounded byte copying, clearing, moving, and string
  comparison.

Build and run it with:

```sh
make example
```

The example is intentionally small enough to read from top to bottom. It is not a
test harness; it is a reference for ownership, allocator lifetimes, and the
shape of normal call sites.
