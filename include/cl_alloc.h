/*
 * cl_alloc.h
 * Purpose: Public allocator interface for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_ALLOC_H
#define CL_ALLOC_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_allocator cl_allocator;

/*
 * Allocator functions receive an opaque context plus explicit size/alignment.
 * Passing the size back on resize/free is intentional: debug allocators can
 * validate ownership contracts without storing hidden global state.
 */
typedef void *(*cl_alloc_fn)(void *ctx, size_t size, size_t align);
typedef void *(*cl_resize_fn)(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align);
typedef void (*cl_free_fn)(void *ctx, void *ptr, size_t size, size_t align);

struct cl_allocator {
    void *ctx;
    cl_alloc_fn alloc;
    cl_resize_fn resize;
    cl_free_fn free;
};

bool cl_is_power_of_two(size_t value);
bool cl_is_valid_align(size_t align);
bool cl_align_up_size(size_t value, size_t align, size_t *out);

void *cl_alloc(cl_allocator *allocator, size_t size, size_t align);
void *cl_resize(
    cl_allocator *allocator,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align);
void cl_free(cl_allocator *allocator, void *ptr, size_t size, size_t align);

cl_allocator cl_system_allocator(void);

typedef struct cl_arena {
    unsigned char *base;
    size_t capacity;
    size_t offset;
} cl_arena;

/* Arena memory is caller-owned; reset/restore invalidates later allocations. */
void cl_arena_init(cl_arena *arena, void *buffer, size_t capacity);
void cl_arena_reset(cl_arena *arena);
size_t cl_arena_mark(const cl_arena *arena);
bool cl_arena_restore(cl_arena *arena, size_t mark);
size_t cl_arena_used(const cl_arena *arena);
size_t cl_arena_remaining(const cl_arena *arena);
cl_allocator cl_arena_allocator(cl_arena *arena);

typedef struct cl_pool {
    unsigned char *state;
    unsigned char *base;
    size_t capacity;
    size_t block_size;
    size_t block_align;
    size_t block_stride;
    size_t block_count;
    size_t free_count;
    void *free_list;
    size_t invalid_free_count;
    size_t mismatch_count;
    size_t double_free_count;
} cl_pool;

/*
 * Pool memory is caller-owned. Each allocation returns one fixed-size block;
 * free returns that block to the pool for reuse.
 */
bool cl_pool_init(
    cl_pool *pool,
    void *buffer,
    size_t capacity,
    size_t block_size,
    size_t block_align);
void cl_pool_reset(cl_pool *pool);
size_t cl_pool_block_count(const cl_pool *pool);
size_t cl_pool_free_count(const cl_pool *pool);
size_t cl_pool_used_count(const cl_pool *pool);
size_t cl_pool_invalid_free_count(const cl_pool *pool);
size_t cl_pool_mismatch_count(const cl_pool *pool);
size_t cl_pool_double_free_count(const cl_pool *pool);
cl_allocator cl_pool_allocator(cl_pool *pool);

typedef struct cl_free_list {
    unsigned char *base;
    size_t capacity;
    size_t free_bytes;
    void *free_list;
    size_t invalid_free_count;
    size_t mismatch_count;
    size_t double_free_count;
} cl_free_list;

/*
 * Free-list memory is caller-owned. Allocations are variable-sized, returned
 * blocks are coalesced with adjacent free blocks, and reset invalidates all
 * outstanding allocations.
 */
bool cl_free_list_init(cl_free_list *list, void *buffer, size_t capacity);
void cl_free_list_reset(cl_free_list *list);
size_t cl_free_list_capacity(const cl_free_list *list);
size_t cl_free_list_free_bytes(const cl_free_list *list);
size_t cl_free_list_used_bytes(const cl_free_list *list);
size_t cl_free_list_invalid_free_count(const cl_free_list *list);
size_t cl_free_list_mismatch_count(const cl_free_list *list);
size_t cl_free_list_double_free_count(const cl_free_list *list);
cl_allocator cl_free_list_allocator(cl_free_list *list);

typedef struct cl_debug_allocator {
    cl_allocator backing;
    void *blocks;
    size_t live_bytes;
    size_t peak_bytes;
    size_t allocation_count;
    size_t free_count;
    size_t failed_count;
    size_t corruption_count;
    size_t mismatch_count;
    size_t double_free_count;
} cl_debug_allocator;

/*
 * Debug allocators quarantine backing allocations until release. This costs
 * memory, but preserves metadata long enough to catch double frees safely.
 */
void cl_debug_allocator_init(cl_debug_allocator *debug, cl_allocator backing);
cl_allocator cl_debug_allocator_view(cl_debug_allocator *debug);
void cl_debug_allocator_release(cl_debug_allocator *debug);

#ifdef __cplusplus
}
#endif

#endif
