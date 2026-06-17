<!--
docs/benchmarks.md
Purpose: Benchmark notes for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# Benchmarks

Benchmarks are built and run with:

```sh
make bench
```

The allocator benchmark compares:

- raw `malloc/free`
- `cl_system_allocator` wrapper overhead
- raw `malloc/realloc/free`
- `cl_system_allocator` resize overhead
- arena allocation with periodic batch reset
- arena allocation with mark/restore
- debug allocator allocation and resize overhead

Rows are reported as elapsed seconds and normalized nanoseconds per operation.
The debug allocator rows include the final quarantine release time, because the
debug allocator intentionally keeps backing allocations alive until
`cl_debug_allocator_release` so it can detect double frees of its own pointers.

## Reading the Numbers

`ns/op` means nanoseconds per operation. Lower is faster. The exact value will
move from run to run, so the main signal is the relationship between rows:

- arena rows should be fastest because they mostly adjust an offset
- `cl_system_allocator` should be close to raw `malloc`/`free`, with some wrapper
  overhead
- debug rows should be much slower because they add guards, metadata, poisoning,
  mismatch checks, and quarantine release

Use the benchmark to catch surprising regressions and to compare allocator
strategies on the same machine. Do not treat one run as a universal performance
claim.

These numbers are useful for rough local comparisons, not as absolute claims
about allocator performance across machines or operating systems.
