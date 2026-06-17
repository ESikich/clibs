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
#include "cl_bench.h"

#include <stdio.h>
#include <stdlib.h>

#define CL_BENCH_FAST_ITERS 1000000u
#define CL_BENCH_RESIZE_ITERS 200000u
#define CL_BENCH_DEBUG_ITERS 100000u
#define CL_BENCH_ARENA_SIZE (64u * 1024u * 1024u)
#define CL_BENCH_POOL_SIZE (1024u * 64u)
#define CL_BENCH_FREE_LIST_SIZE (1024u * 128u)
#define CL_BENCH_BATCH_SIZE 1024u
#define CL_BENCH_BATCH_ROUNDS 1000u

static const size_t cl_bench_mixed_sizes[] = {16u, 32u, 64u, 96u};

static size_t cl_bench_mixed_size(size_t index)
{
    size_t count = sizeof(cl_bench_mixed_sizes) / sizeof(cl_bench_mixed_sizes[0]);

    return cl_bench_mixed_sizes[index % count];
}

static cl_bench_result cl_bench_raw_malloc_free(void)
{
    double start;
    size_t i;

    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = malloc(32u);
        cl_bench_use_ptr(ptr);
        free(ptr);
    }

    return cl_bench_report(
        "raw malloc/free 32B", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_system_alloc_free(void)
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

    return cl_bench_report(
        "cl_system alloc/free 32B", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_raw_malloc_batch_free(void)
{
    void *ptrs[CL_BENCH_BATCH_SIZE];
    double start;
    size_t round;
    size_t i;

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_BATCH_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            ptrs[i] = malloc(32u);
            cl_bench_use_ptr(ptrs[i]);
        }
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            free(ptrs[i]);
        }
    }

    return cl_bench_report(
        "raw malloc batch/free 32B",
        CL_BENCH_BATCH_ROUNDS * CL_BENCH_BATCH_SIZE,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_system_batch_free(void)
{
    cl_allocator allocator = cl_system_allocator();
    void *ptrs[CL_BENCH_BATCH_SIZE];
    double start;
    size_t round;
    size_t i;

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_BATCH_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            ptrs[i] = cl_alloc(&allocator, 32u, 16u);
            cl_bench_use_ptr(ptrs[i]);
        }
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            cl_free(&allocator, ptrs[i], 32u, 16u);
        }
    }

    return cl_bench_report(
        "cl_system batch/free 32B",
        CL_BENCH_BATCH_ROUNDS * CL_BENCH_BATCH_SIZE,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_system_mixed_batch_free(void)
{
    cl_allocator allocator = cl_system_allocator();
    void *ptrs[CL_BENCH_BATCH_SIZE];
    double start;
    size_t round;
    size_t i;

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_BATCH_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            ptrs[i] = cl_alloc(&allocator, cl_bench_mixed_size(i), 16u);
            cl_bench_use_ptr(ptrs[i]);
        }
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            cl_free(&allocator, ptrs[i], cl_bench_mixed_size(i), 16u);
        }
    }

    return cl_bench_report(
        "cl_system mixed batch/free",
        CL_BENCH_BATCH_ROUNDS * CL_BENCH_BATCH_SIZE,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_raw_malloc_realloc_free(void)
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

    return cl_bench_report(
        "raw malloc/realloc/free 32B->96B", CL_BENCH_RESIZE_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_system_resize_free(void)
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

    return cl_bench_report(
        "cl_system resize/free 32B->96B", CL_BENCH_RESIZE_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_arena_batch_reset(void)
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

    return cl_bench_report(
        "arena alloc 32B, reset per batch", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_arena_mark_restore(void)
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

    return cl_bench_report(
        "arena alloc 32B, mark/restore", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_pool_alloc_free(void)
{
    static unsigned char storage[CL_BENCH_POOL_SIZE];
    cl_pool pool;
    cl_allocator allocator;
    double start;
    size_t i;

    if (!cl_pool_init(&pool, storage, sizeof(storage), 32u, 16u)) {
        return cl_bench_report("pool alloc/free 32B", 0u, 0.0);
    }
    allocator = cl_pool_allocator(&pool);

    /*
     * Pool allocation models fixed-size object reuse: pop a free slot, then
     * push it back without touching the general-purpose allocator.
     */
    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 32u, 16u);
    }

    return cl_bench_report(
        "pool alloc/free 32B", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_pool_batch_free(void)
{
    static unsigned char storage[CL_BENCH_POOL_SIZE];
    cl_pool pool;
    cl_allocator allocator;
    void *ptrs[CL_BENCH_BATCH_SIZE];
    double start;
    size_t round;
    size_t i;

    if (!cl_pool_init(&pool, storage, sizeof(storage), 32u, 16u) ||
        cl_pool_block_count(&pool) < CL_BENCH_BATCH_SIZE) {
        return cl_bench_report("pool batch/free 32B", 0u, 0.0);
    }
    allocator = cl_pool_allocator(&pool);

    /*
     * Batch allocation keeps many slots live at once, which is closer to object
     * pool workloads than immediately freeing the same slot.
     */
    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_BATCH_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            ptrs[i] = cl_alloc(&allocator, 32u, 16u);
            cl_bench_use_ptr(ptrs[i]);
        }
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            cl_free(&allocator, ptrs[i], 32u, 16u);
        }
    }

    return cl_bench_report(
        "pool batch/free 32B", CL_BENCH_BATCH_ROUNDS * CL_BENCH_BATCH_SIZE,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_free_list_alloc_free(void)
{
    static unsigned char storage[CL_BENCH_FREE_LIST_SIZE];
    cl_free_list list;
    cl_allocator allocator;
    double start;
    size_t i;

    if (!cl_free_list_init(&list, storage, sizeof(storage))) {
        return cl_bench_report("free-list alloc/free 32B", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&list);

    /*
     * The free-list row models variable-size reuse. It does more bookkeeping
     * than a pool because blocks can split and coalesce.
     */
    start = cl_bench_now_seconds();
    for (i = 0u; i < CL_BENCH_FAST_ITERS; ++i) {
        void *ptr = cl_alloc(&allocator, 32u, 16u);
        cl_bench_use_ptr(ptr);
        cl_free(&allocator, ptr, 32u, 16u);
    }

    return cl_bench_report(
        "free-list alloc/free 32B", CL_BENCH_FAST_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_free_list_mixed_batch_free(void)
{
    static unsigned char storage[CL_BENCH_FREE_LIST_SIZE];
    cl_free_list list;
    cl_allocator allocator;
    void *ptrs[CL_BENCH_BATCH_SIZE];
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&list, storage, sizeof(storage))) {
        return cl_bench_report("free-list mixed batch/free", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&list);

    /*
     * Mixed sizes exercise split and coalesce behavior while keeping the same
     * lifetime pattern as the matching system allocator row.
     */
    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_BATCH_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            ptrs[i] = cl_alloc(&allocator, cl_bench_mixed_size(i), 16u);
            cl_bench_use_ptr(ptrs[i]);
        }
        for (i = 0u; i < CL_BENCH_BATCH_SIZE; ++i) {
            cl_free(&allocator, ptrs[i], cl_bench_mixed_size(i), 16u);
        }
    }

    return cl_bench_report(
        "free-list mixed batch/free",
        CL_BENCH_BATCH_ROUNDS * CL_BENCH_BATCH_SIZE,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_debug_alloc_free(void)
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

    return cl_bench_report(
        "debug wrapper alloc/free 32B", CL_BENCH_DEBUG_ITERS,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_debug_resize_free(void)
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

    return cl_bench_report(
        "debug wrapper resize/free 32B->96B", CL_BENCH_DEBUG_ITERS,
        cl_bench_now_seconds() - start);
}

int main(void)
{
    cl_bench_result raw_alloc;
    cl_bench_result system_alloc;
    cl_bench_result raw_batch;
    cl_bench_result system_batch;
    cl_bench_result system_mixed_batch;
    cl_bench_result raw_resize;
    cl_bench_result system_resize;
    cl_bench_result arena_batch;
    cl_bench_result arena_mark;
    cl_bench_result pool;
    cl_bench_result pool_batch;
    cl_bench_result free_list;
    cl_bench_result free_list_mixed_batch;
    cl_bench_result debug_alloc;
    cl_bench_result debug_resize;

    cl_bench_print_table_header(
        "cl_alloc benchmark",
        "note: debug rows include quarantine release time");

    raw_alloc = cl_bench_raw_malloc_free();
    system_alloc = cl_bench_system_alloc_free();
    raw_batch = cl_bench_raw_malloc_batch_free();
    system_batch = cl_bench_system_batch_free();
    system_mixed_batch = cl_bench_system_mixed_batch_free();
    raw_resize = cl_bench_raw_malloc_realloc_free();
    system_resize = cl_bench_system_resize_free();
    arena_batch = cl_bench_arena_batch_reset();
    arena_mark = cl_bench_arena_mark_restore();
    pool = cl_bench_pool_alloc_free();
    pool_batch = cl_bench_pool_batch_free();
    free_list = cl_bench_free_list_alloc_free();
    free_list_mixed_batch = cl_bench_free_list_mixed_batch_free();
    debug_alloc = cl_bench_debug_alloc_free();
    debug_resize = cl_bench_debug_resize_free();

    putchar('\n');
    puts("interpretation");
    cl_bench_print_ratio("cl_system alloc/free wrapper cost", &system_alloc,
                         &raw_alloc);
    cl_bench_print_ratio("cl_system batch/free wrapper cost", &system_batch,
                         &raw_batch);
    cl_bench_print_ratio("cl_system resize wrapper cost", &system_resize,
                         &raw_resize);
    cl_bench_print_speedup("arena batch allocation", &arena_batch,
                           &system_alloc);
    cl_bench_print_speedup("arena mark/restore allocation", &arena_mark,
                           &system_alloc);
    cl_bench_print_speedup("pool fixed-size reuse", &pool, &system_alloc);
    cl_bench_print_speedup("pool fixed-size batch reuse", &pool_batch,
                           &system_batch);
    cl_bench_print_speedup("free-list variable-size reuse", &free_list,
                           &system_alloc);
    cl_bench_print_speedup("free-list mixed-size batch reuse",
                           &free_list_mixed_batch, &system_mixed_batch);
    cl_bench_print_ratio("debug alloc/free overhead", &debug_alloc,
                         &system_alloc);
    cl_bench_print_ratio("debug resize overhead", &debug_resize,
                         &system_resize);

    putchar('\n');
    printf("sink: %lu\n", cl_bench_sink_value());
    return 0;
}
