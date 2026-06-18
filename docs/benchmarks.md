<!--
docs/benchmarks.md
Purpose: Benchmark notes for clibs.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# Benchmarks

Benchmarks are built and run with:

```sh
make bench
```

The allocator benchmark compares:

- raw `malloc/free`
- `cl_system_allocator` wrapper overhead
- raw and `cl_system_allocator` batch allocate/free behavior
- `cl_system_allocator` mixed-size batch allocate/free behavior
- raw `malloc/realloc/free`
- `cl_system_allocator` resize overhead
- arena allocation with periodic batch reset
- arena allocation with mark/restore
- pool allocation/free for fixed-size blocks
- pool batch allocation/free for fixed-size blocks
- free-list allocation/free for variable-size storage reuse
- free-list mixed-size batch allocation/free
- debug allocator allocation and resize overhead

The container benchmark compares:

- reserved typed array push/pop
- fixed-capacity FIFO queue push/pop
- byte ring buffer push/pop
- intrusive list push/pop
- binary heap heapify plus repeated pop
- fixed-capacity priority queue push/pop
- bitset set/count/clear
- hash table put/get/clear and hot lookup
- hash set insert/contains/clear
- ordered map put/get/clear and hot lookup

Benchmark executables share the header-only `cl_bench` helper for monotonic
timing, table output, ratio reporting, and optimization barriers. Its interface
is documented in [cl_bench.md](cl_bench.md).

Rows are reported as elapsed seconds and normalized nanoseconds per operation.
The debug allocator rows include the final quarantine release time, because the
debug allocator intentionally keeps backing allocations alive until
`cl_debug_allocator_release` so it can detect double frees of its own pointers.

After the raw timing table, the benchmark prints an interpretation block with
local ratios:

- `cl_system` wrapper cost is shown as a multiple of the matching raw
  `malloc`/`free` or `malloc`/`realloc`/`free` row.
- immediate-free arena, pool, and free-list throughput is shown relative to
  `cl_system alloc/free 32B`.
- batch pool throughput is shown relative to `cl_system batch/free 32B`.
- mixed-size free-list throughput is shown relative to
  `cl_system mixed batch/free`.
- debug allocator overhead is shown as a multiple of the matching `cl_system`
  row.
- the container benchmark compares queue, ring, and list push/pop costs against
  reserved array push/pop; compares hash, set, and map mutation workloads;
  compares ordered-map hot lookup against hash hot lookup; and calls out the
  relative cost of bulk bitset operations.

## Reading the Numbers

`ns/op` means nanoseconds per operation. Lower is faster. The exact value will
move from run to run, so the main signal is the relationship between rows:

- arena rows should be fastest because they mostly adjust an offset
- the pool row should be close to arena timings for fixed-size allocate/free
  reuse, but the batch row is a better model for many-live-object pool usage
- the free-list immediate row still pays variable-size bookkeeping cost, while
  the mixed-size batch row should be close to or better than the matching
  `cl_system` row when small-bin reuse is effective
- `cl_system_allocator` should be close to raw `malloc`/`free`, with some wrapper
  overhead
- debug rows should be much slower because they add guards, metadata, poisoning,
  mismatch checks, and quarantine release
- container ratios are intended to point at optimization targets: a high
  mutation ratio usually means allocation, rehashing, rotations, or clearing
  deserves inspection, while a high hot-lookup ratio points at probe length,
  key comparison, tree depth, or cache behavior

In the interpretation block, wrapper and debug ratios are cost multipliers: a
larger number means more overhead than the baseline. Arena, pool, and free-list
ratios are throughput comparisons: a larger number means more operations per
unit time than the matching `cl_system` baseline. Container ratios are also cost
multipliers unless the label explicitly says throughput.

Use the benchmark to catch surprising regressions and to compare allocator
and container strategies on the same machine. Do not treat one run as a
universal performance claim.

These numbers are useful for rough local comparisons, not as absolute claims
about allocator performance across machines or operating systems.
