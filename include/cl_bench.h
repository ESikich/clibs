/*
 * cl_bench.h
 * Purpose: Tiny microbenchmark helpers for clibs benchmarks.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_BENCH_H
#define CL_BENCH_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_bench_result {
    const char *name;
    size_t iterations;
    double seconds;
    double ns_per_op;
} cl_bench_result;

static inline double cl_bench_now_seconds(void)
{
    struct timespec ts;

    /* CLOCK_MONOTONIC avoids wall-clock jumps while staying inside POSIX. */
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0.0;
    }

    return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}

static inline volatile uintptr_t *cl_bench_sink_storage(void)
{
    /*
     * The sink gives benchmarked values an observable use. Without it,
     * optimizing compilers may erase loops that have no visible result.
     */
    static volatile uintptr_t sink;

    return &sink;
}

static inline void cl_bench_use_ptr(const void *ptr)
{
    *cl_bench_sink_storage() ^= (uintptr_t)ptr;
}

static inline unsigned long cl_bench_sink_value(void)
{
    return (unsigned long)*cl_bench_sink_storage();
}

static inline void cl_bench_print_table_header(const char *title, const char *note)
{
    if (title) {
        puts(title);
    }
    puts("target: POSIX.1-2008 C99");
    if (note) {
        puts(note);
    }
    putchar('\n');
    printf("%-42s %12s %12s %12s\n", "case", "iterations", "seconds", "ns/op");
    printf("%-42s %12s %12s %12s\n", "----", "----------", "-------", "-----");
}

static inline cl_bench_result cl_bench_report(
    const char *name,
    size_t iterations,
    double seconds)
{
    cl_bench_result result;
    double ns_per_op = 0.0;

    if (iterations != 0u && seconds > 0.0) {
        ns_per_op = (seconds * 1000000000.0) / (double)iterations;
    }

    printf("%-42s %12zu %12.6f %12.2f\n", name, iterations, seconds, ns_per_op);

    result.name = name;
    result.iterations = iterations;
    result.seconds = seconds;
    result.ns_per_op = ns_per_op;
    return result;
}

static inline void cl_bench_print_ratio(
    const char *label,
    const cl_bench_result *candidate,
    const cl_bench_result *baseline)
{
    if (!candidate || !baseline || candidate->ns_per_op <= 0.0 ||
        baseline->ns_per_op <= 0.0) {
        printf("%-48s unavailable\n", label);
        return;
    }

    printf("%-48s %9.2fx vs %s\n", label,
           candidate->ns_per_op / baseline->ns_per_op, baseline->name);
}

static inline void cl_bench_print_speedup(
    const char *label,
    const cl_bench_result *candidate,
    const cl_bench_result *baseline)
{
    if (!candidate || !baseline || candidate->ns_per_op <= 0.0 ||
        baseline->ns_per_op <= 0.0) {
        printf("%-48s unavailable\n", label);
        return;
    }

    printf("%-48s %9.2fx throughput vs %s\n", label,
           baseline->ns_per_op / candidate->ns_per_op, baseline->name);
}

#ifdef __cplusplus
}
#endif

#endif
