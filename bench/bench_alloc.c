/*
 * bench_alloc.c
 * Purpose: Comparative allocation benchmark for cl_alloc implementations.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "cl_alloc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CL_BENCH_FAST_ITERS 1000000u
#define CL_BENCH_RESIZE_ITERS 200000u
#define CL_BENCH_DEBUG_ITERS 100000u
#define CL_BENCH_ARENA_SIZE (64u * 1024u * 1024u)
#define CL_BENCH_BATCH_SIZE 1024u

/*
 * The sink gives each allocated pointer an observable use. Without it, an
 * optimizing compiler may erase benchmark loops that have no visible result.
 */
static volatile uintptr_t cl_bench_sink;

static double cl_bench_now_seconds(void)
{
    struct timespec ts;

    /* CLOCK_MONOTONIC avoids wall-clock jumps while staying inside POSIX. */
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0.0;
    }

    return (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
}

static void cl_bench_use_ptr(const void *ptr)
{
    cl_bench_sink ^= (uintptr_t)ptr;
}

static void cl_bench_report(const char *name, size_t iterations, double seconds)
{
    double ns_per_op = 0.0;

    if (iterations != 0u && seconds > 0.0) {
        ns_per_op = (seconds * 1000000000.0) / (double)iterations;
    }

    printf("%-42s %12zu %12.6f %12.2f\n", name, iterations, seconds, ns_per_op);
}

static void cl_bench_raw_malloc_free(void)
{
    double start;
    size_t i;

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = malloc(32u);
        cl_bench_use_ptr(ptr);
        free(ptr);
    }

    cl_bench_report("raw malloc/free 32B", CL_BENCH_FAST_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_system_alloc_free(void)
{
    cl_allocator allocator = cl_system_allocator();
    double start;
    size_t i;

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 32u, 16u);
    }

    cl_bench_report("cl_system alloc/free 32B", CL_BENCH_FAST_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_raw_malloc_realloc_free(void)
{
    double start;
    size_t i;

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_RESIZE_ITERS; ++i) {
        void *ptr = malloc(32u);
        void *next;

        if (!ptr) {
            continue;
        }

        next = realloc(ptr, 96u);
        if (!next) {
            free(ptr);
            continue;
        }

        ptr = next;
        cl_bench_use_ptr(ptr);
        free(ptr);
    }

    cl_bench_report("raw malloc/realloc/free 32B->96B", CL_BENCH_RESIZE_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_system_resize_free(void)
{
    cl_allocator allocator = cl_system_allocator();
    double start;
    size_t i;

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_RESIZE_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        void *next;

        if (!ptr) {
            continue;
        }

        next = cl_resize(&allocator, ptr, 32u, 96u, 16u);
        if (!next) {
            cl_free(&allocator, ptr, 32u, 16u);
            continue;
        }

        ptr = next;
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 96u, 16u);
    }

    cl_bench_report("cl_system resize/free 32B->96B", CL_BENCH_RESIZE_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_arena_batch_reset(void)
{
    static unsigned char storage[CL_BENCH_ARENA_SIZE];
    cl_arena arena;
    cl_allocator allocator;
    double start;
    size_t i;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);

    /*
     * This models request/frame-style allocation: many short-lived allocations
     * are discarded together instead of freed one by one.
     */
    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        if ((i + 1u) % CL_BENCH_BATCH_SIZE == 0u) {
            cl_arena_reset(&arena);
        }
    }

    cl_bench_report("arena alloc 32B, reset per batch", CL_BENCH_FAST_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_arena_mark_restore(void)
{
    static unsigned char storage[CL_BENCH_ARENA_SIZE];
    cl_arena arena;
    cl_allocator allocator;
    double start;
    size_t i;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);

    /*
     * Mark/restore measures temporary allocation scopes, where each allocation
     * is undone immediately after the short-lived work finishes.
     */
    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        size_t mark = cl_arena_mark(&arena);
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        (void)cl_arena_restore(&arena, mark);
    }

    cl_bench_report("arena alloc 32B, mark/restore", CL_BENCH_FAST_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_debug_alloc_free(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    double start;
    size_t i;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    /*
     * Debug timings intentionally include release. The allocator quarantines
     * freed blocks to catch double frees, and release is when backing memory is
     * actually returned.
     */
    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_DEBUG_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 32u, 16u);
    }
    cl_debug_allocator_release(&debug);

    cl_bench_report("debug wrapper alloc/free 32B", CL_BENCH_DEBUG_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_debug_resize_free(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    double start;
    size_t i;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_DEBUG_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        void *next;

        if (!ptr) {
            continue;
        }

        next = cl_resize(&allocator, ptr, 32u, 96u, 16u);
        if (!next) {
            cl_free(&allocator, ptr, 32u, 16u);
            continue;
        }

        ptr = next;
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 96u, 16u);
    }
    cl_debug_allocator_release(&debug);

    cl_bench_report("debug wrapper resize/free 32B->96B", CL_BENCH_DEBUG_ITERS,
                    cl_bench_now_seconds() - start);
}

static void cl_bench_print_header(void)
{
    puts("cl_alloc benchmark");
    puts("target: POSIX.1-2008 C99");
    puts("note: debug rows include quarantine release time");
    putchar('\n');
    printf("%-42s %12s %12s %12s\n", "case", "iterations", "seconds", "ns/op");
    printf("%-42s %12s %12s %12s\n", "----", "----------", "-------", "-----");
}

int main(void)
{
    cl_bench_print_header();

    cl_bench_raw_malloc_free();
    cl_bench_system_alloc_free();
    cl_bench_raw_malloc_realloc_free();
    cl_bench_system_resize_free();
    cl_bench_arena_batch_reset();
    cl_bench_arena_mark_restore();
    cl_bench_debug_alloc_free();
    cl_bench_debug_resize_free();

    putchar('\n');
    printf("sink: %lu\n", (unsigned long)cl_bench_sink);
    return 0;
}
