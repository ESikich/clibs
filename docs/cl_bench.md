<!--
cl_bench.md
Purpose: Microbenchmark helper documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_bench

`cl_bench` is a tiny header-only helper for repository microbenchmarks. It keeps
benchmark programs explicit C99 while sharing common timing, result reporting,
ratio reporting, and optimization barriers.

## Interface

Use `cl_bench_now_seconds` around the code being measured, then report the
elapsed time with `cl_bench_report`:

```c
double start = cl_bench_now_seconds();
size_t i;

for (i = 0u; i < iterations; ++i) {
    void *ptr = work();
    cl_bench_use_ptr(ptr);
}

result = cl_bench_report(
    "example work", iterations, cl_bench_now_seconds() - start);
```

`cl_bench_print_table_header` prints the common benchmark heading. The returned
`cl_bench_result` can be passed to `cl_bench_print_ratio` for cost multipliers
or `cl_bench_print_speedup` for throughput comparisons.

Call `cl_bench_sink_value` near the end of a benchmark executable when measured
loops feed values into `cl_bench_use_ptr`. Printing the sink makes those pointer
uses externally visible enough that optimizing compilers should not erase the
work.

## Scope

The helper uses `clock_gettime(CLOCK_MONOTONIC)` and therefore expects the
project's POSIX.1-2008 feature-test macro setup. It is meant for local
microbenchmarks and regression checks, not for making cross-machine performance
claims.
