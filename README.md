<!--
README.md
Purpose: Project overview and quick-start instructions.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# clibs

Low-level C libraries with memory safety and performance as the first constraints.

This project targets POSIX.1-2008 compatible systems and C99. Keep new code inside
that portability envelope unless a later project decision changes it.

## Current libraries

`cl_alloc` provides a small explicit allocator interface:

- system allocator wrapper
- arena allocator with mark/restore/reset
- fixed-size pool allocator over caller-owned storage
- variable-size free-list allocator over caller-owned storage
- debug allocator wrapper with guard bytes, size/alignment mismatch detection, double-free detection for its own pointers, live byte counters, and peak byte counters

Pool and free-list allocators also expose counters for invalid frees,
size/alignment mismatches, and double-free attempts.

`cl_atomic` provides thin C99-friendly wrappers around GCC/Clang `__atomic`
builtins for integer and pointer state with explicit memory orders.

The test suite uses `cl_test`, a tiny header-only unit test helper for C99 test
programs. Benchmarks use `cl_bench`, a matching header-only helper for monotonic
timing and compact benchmark reporting.

`cl_libc` provides a tiny prefixed subset of libc-style memory and string
primitives: `cl_memset`, `cl_memcpy`, `cl_memmove`, `cl_memcmp`, `cl_strlen`,
`cl_strcmp`, and `cl_strchr`.

`cl_list` provides allocation-free intrusive doubly linked lists with O(1)
push, pop, insert, and remove operations over caller-owned objects.

`cl_map` provides an allocator-backed ordered map for non-owning byte keys with
caller-owned pointer values and byte-lexicographic iteration.

`cl_sv` provides non-owning byte string views with trimming, comparison,
delimiter splitting, and checked decimal integer parsing.

`cl_array` provides macro-generated typed dynamic arrays backed by `cl_allocator`
with checked capacity growth, reserve/resize/push/pop operations, and
zero-initialized growth through `resize`.

`cl_ascii` provides locale-free ASCII classification, case conversion, and
digit/hex value helpers.

`cl_bitset` provides fixed-storage and allocator-backed bitsets with
range-checked bit access, boolean operations, population counts, and
first-set/first-clear scans.

`cl_buffer` provides owned growable byte buffers backed by `cl_allocator` plus
caller-owned ring buffers for bounded FIFO byte streams.

`cl_endian` provides byte-order conversion, byte swaps, and unaligned
load/store helpers for fixed-width unsigned integers.

`cl_file` provides POSIX whole-file read/write helpers that use `cl_allocator`
for owned read buffers, plus explicit streaming open/read/write/append helpers.
Whole-file reads preserve binary file sizes while adding a convenience NUL
terminator after the read bytes.

`cl_hash` provides FNV-1a 64-bit hashing and an allocator-backed hash table for
non-owning byte keys with caller-owned pointer values.

`cl_heap` provides allocation-free binary heap algorithms over caller-owned
arrays, including heapify, push, pop, validation, and in-place heap sort.

`cl_set` provides an allocator-backed hash set for non-owning byte keys without
requiring placeholder values.

`cl_path` provides lexical POSIX path normalization, joining, basename, and
dirname helpers over caller-owned buffers and non-owning views.

`cl_priority_queue` provides allocation-free fixed-capacity binary priority
queues over caller-owned storage and comparator-defined element ordering.

`cl_queue` provides allocation-free fixed-capacity FIFO ring queues over
caller-owned storage and caller-specified element sizes.

`cl_time` provides monotonic timestamps, checked duration math, and simple
elapsed timer helpers over signed 64-bit nanosecond values.

`cl_utf8` provides allocation-free UTF-8 validation, decoding, encoding, and
iteration over caller-owned byte spans.

Build and test:

```sh
make test
```

Run the comparative allocation and container benchmarks:

```sh
make bench
```

The benchmark compares raw allocation APIs, allocator wrapper overhead, arena
lifetime patterns, pool and free-list reuse, debug allocator overhead, and core
container operation costs. Notes live in
[docs/benchmarks.md](docs/benchmarks.md).

Run the overview example:

```sh
make example
```

The example in [examples/overview.c](examples/overview.c) parses records with
`cl_sv`, stores them in a `cl_array`, backs allocations with `cl_alloc`, uses
`cl_hash` for name lookups, uses `cl_atomic` to publish a count, uses
`cl_bitset` to mark selected records, uses `cl_buffer` for bounded byte-stream
handling, uses `cl_endian` for explicit binary byte order, uses `cl_heap` for
in-place heap ordering over caller-owned arrays, uses `cl_set` to record unique
names, uses `cl_map` for ordered name lookup, uses `cl_list` for allocation-free
event ordering, uses `cl_queue` for caller-owned FIFO storage, uses
`cl_priority_queue` for comparator-ordered caller-owned storage, uses `cl_path`
for lexical path handling, uses `cl_time` to measure elapsed runtime, uses
`cl_utf8` for input validation, uses `cl_ascii` for locale-free byte
classification, and uses `cl_libc` helpers for bounded byte and string
operations. Notes live in [docs/examples.md](docs/examples.md).

Project conventions live in [AGENTS.md](AGENTS.md). Documentation lives in
[docs/](docs/) and should be updated alongside code changes. The project
roadmap lives in [TODO.md](TODO.md).
