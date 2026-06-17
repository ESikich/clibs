/*
 * test_alloc.c
 * Purpose: Safety and behavior tests for cl_alloc.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_alloc.h"
#include "cl_test.h"

#include <stdint.h>
#include <string.h>

static int test_align_helpers(void)
{
    size_t out = 0u;

    CL_TEST_CHECK(!cl_is_power_of_two(0u));
    CL_TEST_CHECK(cl_is_power_of_two(1u));
    CL_TEST_CHECK(cl_is_power_of_two(64u));
    CL_TEST_CHECK(!cl_is_power_of_two(96u));
    CL_TEST_CHECK(cl_align_up_size(17u, 8u, &out));
    CL_TEST_CHECK(out == 24u);
    CL_TEST_CHECK(!cl_align_up_size((size_t)-4, 8u, &out));
    CL_TEST_CHECK(!cl_align_up_size(8u, 0u, &out));

    return 0;
}

static int test_system_allocator(void)
{
    cl_allocator system = cl_system_allocator();
    unsigned char *ptr;
    unsigned char *grown;
    size_t i;

    ptr = cl_alloc(&system, 64u, 64u);
    CL_TEST_CHECK(ptr != NULL);
    CL_TEST_CHECK(((uintptr_t)ptr & 63u) == 0u);

    for (i = 0u; i < 64u; ++i) {
        ptr[i] = (unsigned char)i;
    }

    grown = cl_resize(&system, ptr, 64u, 128u, 64u);
    CL_TEST_CHECK(grown != NULL);
    CL_TEST_CHECK(((uintptr_t)grown & 63u) == 0u);
    for (i = 0u; i < 64u; ++i) {
        CL_TEST_CHECK(grown[i] == (unsigned char)i);
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
    CL_TEST_CHECK(a != NULL);
    CL_TEST_CHECK(((uintptr_t)a & 7u) == 0u);

    mark = cl_arena_mark(&arena);

    b = cl_alloc(&allocator, 32u, 32u);
    CL_TEST_CHECK(b != NULL);
    CL_TEST_CHECK(((uintptr_t)b & 31u) == 0u);
    CL_TEST_CHECK(cl_arena_used(&arena) <= sizeof(storage));

    CL_TEST_CHECK(cl_arena_restore(&arena, mark));
    c = cl_alloc(&allocator, 32u, 32u);
    CL_TEST_CHECK(c == b);
    CL_TEST_CHECK(!cl_arena_restore(&arena, cl_arena_used(&arena) + 1u));

    CL_TEST_CHECK(cl_alloc(&allocator, sizeof(storage) * 2u, 8u) == NULL);
    cl_arena_reset(&arena);
    CL_TEST_CHECK(cl_arena_used(&arena) == 0u);

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
    CL_TEST_CHECK(ptr != NULL);
    for (i = 0u; i < 16u; ++i) {
        ptr[i] = (unsigned char)(i + 1u);
    }

    /* The most recent arena allocation should grow in place when capacity allows. */
    grown = cl_resize(&allocator, ptr, 16u, 32u, 8u);
    CL_TEST_CHECK(grown == ptr);
    for (i = 0u; i < 16u; ++i) {
        CL_TEST_CHECK(grown[i] == (unsigned char)(i + 1u));
    }

    blocker = cl_alloc(&allocator, 16u, 8u);
    CL_TEST_CHECK(blocker != NULL);
    /* Older arena allocations cannot grow in place once another block follows them. */
    moved = cl_resize(&allocator, grown, 32u, 48u, 8u);
    CL_TEST_CHECK(moved != NULL);
    CL_TEST_CHECK(moved != grown);
    for (i = 0u; i < 16u; ++i) {
        CL_TEST_CHECK(moved[i] == (unsigned char)(i + 1u));
    }

    return 0;
}

static int test_pool_allocator_reuses_blocks(void)
{
    unsigned char storage[256];
    cl_pool pool;
    cl_allocator allocator;
    void *a;
    void *b;
    void *c;

    CL_TEST_CHECK(cl_pool_init(&pool, storage, sizeof(storage), 24u, 16u));
    allocator = cl_pool_allocator(&pool);
    CL_TEST_CHECK(cl_pool_block_count(&pool) > 0u);
    CL_TEST_CHECK(cl_pool_free_count(&pool) == cl_pool_block_count(&pool));

    a = cl_alloc(&allocator, 24u, 16u);
    b = cl_alloc(&allocator, 8u, 8u);
    CL_TEST_CHECK(a != NULL);
    CL_TEST_CHECK(b != NULL);
    CL_TEST_CHECK(a != b);
    CL_TEST_CHECK(((uintptr_t)a & 15u) == 0u);
    CL_TEST_CHECK(((uintptr_t)b & 7u) == 0u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 2u);

    cl_free(&allocator, a, 24u, 16u);
    CL_TEST_CHECK(cl_pool_free_count(&pool) == cl_pool_block_count(&pool) - 1u);

    c = cl_alloc(&allocator, 24u, 16u);
    CL_TEST_CHECK(c == a);

    cl_free(&allocator, b, 8u, 8u);
    cl_free(&allocator, c, 24u, 16u);
    CL_TEST_CHECK(cl_pool_free_count(&pool) == cl_pool_block_count(&pool));

    return 0;
}

static int test_pool_allocator_exhaustion_and_reset(void)
{
    unsigned char storage[128];
    cl_pool pool;
    cl_allocator allocator;
    void *blocks[8];
    size_t count;
    size_t i;

    CL_TEST_CHECK(cl_pool_init(&pool, storage, sizeof(storage), 16u, 8u));
    allocator = cl_pool_allocator(&pool);
    count = cl_pool_block_count(&pool);
    CL_TEST_CHECK(count > 0u);
    CL_TEST_CHECK(count <= 8u);

    for (i = 0u; i < count; ++i) {
        blocks[i] = cl_alloc(&allocator, 16u, 8u);
        CL_TEST_CHECK(blocks[i] != NULL);
    }
    CL_TEST_CHECK(cl_alloc(&allocator, 16u, 8u) == NULL);
    CL_TEST_CHECK(cl_pool_free_count(&pool) == 0u);

    cl_pool_reset(&pool);
    CL_TEST_CHECK(cl_pool_free_count(&pool) == count);
    CL_TEST_CHECK(cl_alloc(&allocator, 16u, 8u) == blocks[0]);

    return 0;
}

static int test_pool_allocator_rejects_bad_requests(void)
{
    unsigned char storage[128];
    unsigned char foreign[32];
    cl_pool pool;
    cl_allocator allocator;
    void *ptr;
    void *grown;

    CL_TEST_CHECK(!cl_pool_init(&pool, storage, sizeof(storage), 16u, 3u));
    CL_TEST_CHECK(cl_pool_init(&pool, storage, sizeof(storage), 16u, 8u));
    allocator = cl_pool_allocator(&pool);

    CL_TEST_CHECK(cl_alloc(&allocator, 17u, 8u) == NULL);
    CL_TEST_CHECK(cl_alloc(&allocator, 8u, 16u) == NULL);

    ptr = cl_alloc(&allocator, 16u, 8u);
    CL_TEST_CHECK(ptr != NULL);

    grown = cl_resize(&allocator, ptr, 16u, 16u, 8u);
    CL_TEST_CHECK(grown == ptr);
    CL_TEST_CHECK(cl_resize(&allocator, ptr, 16u, 24u, 8u) == NULL);
    CL_TEST_CHECK(cl_pool_mismatch_count(&pool) == 1u);

    cl_free(&allocator, foreign, 16u, 8u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 1u);
    CL_TEST_CHECK(cl_pool_invalid_free_count(&pool) == 1u);

    cl_free(&allocator, (unsigned char *)ptr + 1u, 16u, 8u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 1u);
    CL_TEST_CHECK(cl_pool_invalid_free_count(&pool) == 2u);

    cl_free(&allocator, ptr, 8u, 16u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 1u);
    CL_TEST_CHECK(cl_pool_mismatch_count(&pool) == 2u);

    cl_free(&allocator, ptr, 16u, 8u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 0u);
    cl_free(&allocator, ptr, 16u, 8u);
    CL_TEST_CHECK(cl_pool_used_count(&pool) == 0u);
    CL_TEST_CHECK(cl_pool_double_free_count(&pool) == 1u);

    return 0;
}

static int test_free_list_allocator_reuses_and_coalesces(void)
{
    unsigned char storage[512];
    cl_free_list list;
    cl_allocator allocator;
    void *a;
    void *b;
    void *c;
    size_t capacity;

    CL_TEST_CHECK(cl_free_list_init(&list, storage, sizeof(storage)));
    allocator = cl_free_list_allocator(&list);
    capacity = cl_free_list_capacity(&list);
    CL_TEST_CHECK(capacity > 0u);
    CL_TEST_CHECK(cl_free_list_free_bytes(&list) == capacity);

    a = cl_alloc(&allocator, 64u, 16u);
    b = cl_alloc(&allocator, 80u, 32u);
    CL_TEST_CHECK(a != NULL);
    CL_TEST_CHECK(b != NULL);
    CL_TEST_CHECK(a != b);
    CL_TEST_CHECK(((uintptr_t)a & 15u) == 0u);
    CL_TEST_CHECK(((uintptr_t)b & 31u) == 0u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) > 144u);

    cl_free(&allocator, a, 64u, 16u);
    cl_free(&allocator, b, 80u, 32u);
    CL_TEST_CHECK(cl_free_list_free_bytes(&list) == capacity);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == 0u);

    c = cl_alloc(&allocator, 160u, 16u);
    CL_TEST_CHECK(c != NULL);
    cl_free(&allocator, c, 160u, 16u);
    CL_TEST_CHECK(cl_free_list_free_bytes(&list) == capacity);

    return 0;
}

static int test_free_list_allocator_resize_preserves_data(void)
{
    unsigned char storage[512];
    cl_free_list list;
    cl_allocator allocator;
    unsigned char *ptr;
    unsigned char *grown;
    unsigned char *shrunk;
    size_t i;

    CL_TEST_CHECK(cl_free_list_init(&list, storage, sizeof(storage)));
    allocator = cl_free_list_allocator(&list);

    ptr = cl_alloc(&allocator, 32u, 16u);
    CL_TEST_CHECK(ptr != NULL);
    for (i = 0u; i < 32u; ++i) {
        ptr[i] = (unsigned char)(0x40u + i);
    }

    grown = cl_resize(&allocator, ptr, 32u, 96u, 16u);
    CL_TEST_CHECK(grown != NULL);
    for (i = 0u; i < 32u; ++i) {
        CL_TEST_CHECK(grown[i] == (unsigned char)(0x40u + i));
    }

    shrunk = cl_resize(&allocator, grown, 96u, 24u, 16u);
    CL_TEST_CHECK(shrunk == grown);
    for (i = 0u; i < 24u; ++i) {
        CL_TEST_CHECK(shrunk[i] == (unsigned char)(0x40u + i));
    }

    cl_free(&allocator, shrunk, 24u, 16u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == 0u);

    return 0;
}

static int test_free_list_allocator_rejects_bad_requests(void)
{
    unsigned char storage[256];
    unsigned char foreign[32];
    cl_free_list list;
    cl_allocator allocator;
    void *ptr;
    size_t used;

    CL_TEST_CHECK(!cl_free_list_init(&list, storage, 1u));
    CL_TEST_CHECK(cl_free_list_init(&list, storage, sizeof(storage)));
    allocator = cl_free_list_allocator(&list);

    CL_TEST_CHECK(cl_alloc(&allocator, 16u, 3u) == NULL);
    CL_TEST_CHECK(cl_alloc(&allocator, sizeof(storage) * 2u, 8u) == NULL);

    ptr = cl_alloc(&allocator, 48u, 8u);
    CL_TEST_CHECK(ptr != NULL);
    used = cl_free_list_used_bytes(&list);

    cl_free(&allocator, foreign, 48u, 8u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == used);
    CL_TEST_CHECK(cl_free_list_invalid_free_count(&list) == 1u);

    cl_free(&allocator, ptr, 32u, 8u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == used);
    CL_TEST_CHECK(cl_free_list_mismatch_count(&list) == 1u);

    cl_free(&allocator, ptr, 48u, 16u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == used);
    CL_TEST_CHECK(cl_free_list_mismatch_count(&list) == 2u);

    cl_free(&allocator, ptr, 48u, 8u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == 0u);
    cl_free(&allocator, ptr, 48u, 8u);
    CL_TEST_CHECK(cl_free_list_used_bytes(&list) == 0u);
    CL_TEST_CHECK(cl_free_list_double_free_count(&list) == 1u);

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
    CL_TEST_CHECK(ptr != NULL);
    CL_TEST_CHECK(((uintptr_t)ptr & 15u) == 0u);
    CL_TEST_CHECK(debug.live_bytes == 32u);
    CL_TEST_CHECK(debug.peak_bytes == 32u);
    CL_TEST_CHECK(debug.allocation_count == 1u);

    cl_free(&allocator, ptr, 32u, 16u);
    CL_TEST_CHECK(debug.live_bytes == 0u);
    CL_TEST_CHECK(debug.free_count == 1u);
    CL_TEST_CHECK(debug.corruption_count == 0u);
    CL_TEST_CHECK(debug.mismatch_count == 0u);

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
    CL_TEST_CHECK(ptr != NULL);
    /* One byte past the user allocation should trip the right guard. */
    ptr[8] = 0xFFu;
    cl_free(&allocator, ptr, 8u, 8u);
    CL_TEST_CHECK(debug.corruption_count == 1u);
    CL_TEST_CHECK(debug.free_count == 1u);

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
    CL_TEST_CHECK(ptr != NULL);
    /* The debug wrapper treats size/alignment mismatches as ownership bugs. */
    cl_free(&allocator, ptr, 16u, 8u);
    CL_TEST_CHECK(debug.mismatch_count == 1u);

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
    CL_TEST_CHECK(ptr != NULL);
    cl_free(&allocator, ptr, 24u, 8u);
    cl_free(&allocator, ptr, 24u, 8u);
    CL_TEST_CHECK(debug.free_count == 1u);
    CL_TEST_CHECK(debug.double_free_count == 1u);

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
    CL_TEST_CHECK(ptr != NULL);
    for (i = 0u; i < 16u; ++i) {
        ptr[i] = (unsigned char)(0x80u + i);
    }

    grown = cl_resize(&allocator, ptr, 16u, 64u, 8u);
    CL_TEST_CHECK(grown != NULL);
    for (i = 0u; i < 16u; ++i) {
        CL_TEST_CHECK(grown[i] == (unsigned char)(0x80u + i));
    }
    CL_TEST_CHECK(debug.live_bytes == 64u);

    cl_free(&allocator, grown, 64u, 8u);
    CL_TEST_CHECK(debug.live_bytes == 0u);

    cl_debug_allocator_release(&debug);
    return 0;
}

int main(void)
{
    static const cl_test_case tests[] = {
        CL_TEST_CASE(test_align_helpers),
        CL_TEST_CASE(test_system_allocator),
        CL_TEST_CASE(test_arena_allocator),
        CL_TEST_CASE(test_arena_resize),
        CL_TEST_CASE(test_pool_allocator_reuses_blocks),
        CL_TEST_CASE(test_pool_allocator_exhaustion_and_reset),
        CL_TEST_CASE(test_pool_allocator_rejects_bad_requests),
        CL_TEST_CASE(test_free_list_allocator_reuses_and_coalesces),
        CL_TEST_CASE(test_free_list_allocator_resize_preserves_data),
        CL_TEST_CASE(test_free_list_allocator_rejects_bad_requests),
        CL_TEST_CASE(test_debug_allocator_counts),
        CL_TEST_CASE(test_debug_allocator_detects_overrun),
        CL_TEST_CASE(test_debug_allocator_detects_mismatch),
        CL_TEST_CASE(test_debug_allocator_detects_double_free),
        CL_TEST_CASE(test_debug_resize_preserves_data)
    };

    return cl_test_run_all("alloc", tests, sizeof(tests) / sizeof(tests[0]));
}
