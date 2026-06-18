<!--
examples.md
Purpose: Example program guide for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# Examples

The overview example in `examples/overview.c` shows the current core libraries
working together in one small program:

- `cl_sv` trims, splits, and parses a comma-separated inventory string.
- `cl_array` stores parsed records in a typed dynamic array.
- `cl_alloc` supplies a free-list allocator wrapped by the debug allocator, plus
  arena scratch storage and a fixed-size pool allocation.
- `cl_hash` maps item names to parsed records without taking ownership of keys.
- `cl_list` queues a pool-allocated event through an embedded intrusive node.
- `cl_atomic` publishes the compacted record count through an atomic size value.
- `cl_bitset` marks selected parsed records with fixed caller-owned storage.
- `cl_buffer` demonstrates a caller-owned ring buffer for bounded byte-stream
  handling.
- `cl_endian` stores and reloads the total count through an explicit big-endian
  byte buffer.
- `cl_file` writes the example input to a temporary file and reads it back into
  allocator-owned storage before parsing.
- `cl_path` joins and inspects a lexical POSIX path without filesystem lookup.
- `cl_time` measures elapsed runtime with a monotonic timer.
- `cl_utf8` validates and counts code points in the input byte span.
- `cl_ascii` checks record names with locale-free byte classification.
- `cl_libc` handles bounded byte copying, clearing, moving, and string
  comparison.

Build and run it with:

```sh
make example
```

The example is intentionally small enough to read from top to bottom. It is not a
test harness; it is a reference for ownership, allocator lifetimes, and the
shape of normal call sites.
