<!--
README.md
Purpose: Project overview and quick-start instructions.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# clibs

Low-level C libraries with memory safety and performance as the first constraints.

This project targets POSIX.1-2008 compatible systems and C99. Keep new code inside
that portability envelope unless a later project decision changes it.

## Current library

`cl_alloc` provides a small explicit allocator interface:

- system allocator wrapper
- arena allocator with mark/restore/reset
- fixed-size pool allocator over caller-owned storage
- variable-size free-list allocator over caller-owned storage
- debug allocator wrapper with guard bytes, size/alignment mismatch detection, double-free detection for its own pointers, live byte counters, and peak byte counters

Pool and free-list allocators also expose counters for invalid frees,
size/alignment mismatches, and double-free attempts.

The test suite uses `cl_test`, a tiny header-only unit test helper for C99 test
programs. Benchmarks use `cl_bench`, a matching header-only helper for monotonic
timing and compact benchmark reporting.

Build and test:

```sh
make test
```

Run the comparative allocation benchmark:

```sh
make bench
```

The benchmark compares raw allocation APIs, allocator wrapper overhead, arena
lifetime patterns, pool and free-list reuse, and debug allocator overhead. Notes
live in [docs/benchmarks.md](docs/benchmarks.md).

Project conventions live in [AGENTS.md](AGENTS.md). Documentation lives in
[docs/](docs/) and should be updated alongside code changes. The project
roadmap lives in [TODO.md](TODO.md).
