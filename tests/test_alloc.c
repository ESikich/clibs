/*
 * test_alloc.c
 * Purpose: Safety and behavior tests for cl_alloc.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_alloc.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr)                                                             \
    do {                                                                        \
        if (!(expr)) {                                                          \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);    \
            return 1;                                                           \
        }                                                                       \
    } while (0)

static int test_align_helpers(void)
{
    size_t out = 0u;

    CHECK(!cl_is_power_of_two(0u));
    CHECK(cl_is_power_of_two(1u));
    CHECK(cl_is_power_of_two(64u));
    CHECK(!cl_is_power_of_two(96u));
    CHECK(cl_align_up_size(17u, 8u, &out));
    CHECK(out == 24u);
    CHECK(!cl_align_up_size((size_t)-4, 8u, &out));
    CHECK(!cl_align_up_size(8u, 0u, &out));

    return 0;
}

static int test_system_allocator(void)
{
    cl_allocator system = cl_system_allocator();
    unsigned char *ptr;
    unsigned char *grown;
    size_t i;

    ptr = cl_alloc(&system, 64u, 64u);
    CHECK(ptr != NULL);
    CHECK(((uintptr_t)ptr & 63u) == 0u);

    for (i = 0u; i < 64u; ++i) {
        ptr[i] = (unsigned char)i;
    }

    grown = cl_resize(&system, ptr, 64u, 128u, 64u);
    CHECK(grown != NULL);
    CHECK(((uintptr_t)grown & 63u) == 0u);
    for (i = 0u; i < 64u; ++i) {
        CHECK(grown[i] == (unsigned char)i);
    }

    cl_free(&system, grown, 128u, 64u);
    return 0;
}

static int test_arena_allocator(void)
{
    unsigned char storage[256];
    cl_arena arena;
    cl_allocator allocator;
    void *a;
    void *b;
    void *c;
    size_t mark;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);

    a = cl_alloc(&allocator, 13u, 8u);
    CHECK(a != NULL);
    CHECK(((uintptr_t)a & 7u) == 0u);

    mark = cl_arena_mark(&arena);

    b = cl_alloc(&allocator, 32u, 32u);
    CHECK(b != NULL);
    CHECK(((uintptr_t)b & 31u) == 0u);
    CHECK(cl_arena_used(&arena) <= sizeof(storage));

    CHECK(cl_arena_restore(&arena, mark));
    c = cl_alloc(&allocator, 32u, 32u);
    CHECK(c == b);
    CHECK(!cl_arena_restore(&arena, cl_arena_used(&arena) + 1u));

    CHECK(cl_alloc(&allocator, sizeof(storage) * 2u, 8u) == NULL);
    cl_arena_reset(&arena);
    CHECK(cl_arena_used(&arena) == 0u);

    return 0;
}

static int test_arena_resize(void)
{
    unsigned char storage[256];
    cl_arena arena;
    cl_allocator allocator;
    unsigned char *ptr;
    unsigned char *grown;
    unsigned char *moved;
    void *blocker;
    size_t i;

    cl_arena_init(&arena, storage, sizeof(storage));
    allocator = cl_arena_allocator(&arena);

    ptr = cl_alloc(&allocator, 16u, 8u);
    CHECK(ptr != NULL);
    for (i = 0u; i < 16u; ++i) {
        ptr[i] = (unsigned char)(i + 1u);
    }

    grown = cl_resize(&allocator, ptr, 16u, 32u, 8u);
    CHECK(grown == ptr);
    for (i = 0u; i < 16u; ++i) {
        CHECK(grown[i] == (unsigned char)(i + 1u));
    }

    blocker = cl_alloc(&allocator, 16u, 8u);
    CHECK(blocker != NULL);
    moved = cl_resize(&allocator, grown, 32u, 48u, 8u);
    CHECK(moved != NULL);
    CHECK(moved != grown);
    for (i = 0u; i < 16u; ++i) {
        CHECK(moved[i] == (unsigned char)(i + 1u));
    }

    return 0;
}

static int test_debug_allocator_counts(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    unsigned char *ptr;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    ptr = cl_alloc(&allocator, 32u, 16u);
    CHECK(ptr != NULL);
    CHECK(((uintptr_t)ptr & 15u) == 0u);
    CHECK(debug.live_bytes == 32u);
    CHECK(debug.peak_bytes == 32u);
    CHECK(debug.allocation_count == 1u);

    cl_free(&allocator, ptr, 32u, 16u);
    CHECK(debug.live_bytes == 0u);
    CHECK(debug.free_count == 1u);
    CHECK(debug.corruption_count == 0u);
    CHECK(debug.mismatch_count == 0u);

    cl_debug_allocator_release(&debug);
    return 0;
}

static int test_debug_allocator_detects_overrun(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    unsigned char *ptr;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    ptr = cl_alloc(&allocator, 8u, 8u);
    CHECK(ptr != NULL);
    ptr[8] = 0xFFu;
    cl_free(&allocator, ptr, 8u, 8u);
    CHECK(debug.corruption_count == 1u);
    CHECK(debug.free_count == 1u);

    cl_debug_allocator_release(&debug);
    return 0;
}

static int test_debug_allocator_detects_mismatch(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    void *ptr;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    ptr = cl_alloc(&allocator, 24u, 8u);
    CHECK(ptr != NULL);
    cl_free(&allocator, ptr, 16u, 8u);
    CHECK(debug.mismatch_count == 1u);

    cl_debug_allocator_release(&debug);
    return 0;
}

static int test_debug_allocator_detects_double_free(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    void *ptr;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    ptr = cl_alloc(&allocator, 24u, 8u);
    CHECK(ptr != NULL);
    cl_free(&allocator, ptr, 24u, 8u);
    cl_free(&allocator, ptr, 24u, 8u);
    CHECK(debug.free_count == 1u);
    CHECK(debug.double_free_count == 1u);

    cl_debug_allocator_release(&debug);
    return 0;
}

static int test_debug_resize_preserves_data(void)
{
    cl_allocator system = cl_system_allocator();
    cl_debug_allocator debug;
    cl_allocator allocator;
    unsigned char *ptr;
    unsigned char *grown;
    size_t i;

    cl_debug_allocator_init(&debug, system);
    allocator = cl_debug_allocator_view(&debug);

    ptr = cl_alloc(&allocator, 16u, 8u);
    CHECK(ptr != NULL);
    for (i = 0u; i < 16u; ++i) {
        ptr[i] = (unsigned char)(0x80u + i);
    }

    grown = cl_resize(&allocator, ptr, 16u, 64u, 8u);
    CHECK(grown != NULL);
    for (i = 0u; i < 16u; ++i) {
        CHECK(grown[i] == (unsigned char)(0x80u + i));
    }
    CHECK(debug.live_bytes == 64u);

    cl_free(&allocator, grown, 64u, 8u);
    CHECK(debug.live_bytes == 0u);

    cl_debug_allocator_release(&debug);
    return 0;
}

int main(void)
{
    CHECK(test_align_helpers() == 0);
    CHECK(test_system_allocator() == 0);
    CHECK(test_arena_allocator() == 0);
    CHECK(test_arena_resize() == 0);
    CHECK(test_debug_allocator_counts() == 0);
    CHECK(test_debug_allocator_detects_overrun() == 0);
    CHECK(test_debug_allocator_detects_mismatch() == 0);
    CHECK(test_debug_allocator_detects_double_free() == 0);
    CHECK(test_debug_resize_preserves_data() == 0);

    puts("alloc tests passed");
    return 0;
}
