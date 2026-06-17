/*
 * cl_alloc.c
 * Purpose: Allocator implementations for system, arena, pool, free-list, and debug allocation.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "cl_alloc.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CL_DEBUG_MAGIC 0xC10CA110C0FFEE11ull
#define CL_DEBUG_FREED 0xFEEEFEEEFEEEFEEFull
#define CL_DEBUG_GUARD_SIZE 16u
#define CL_DEBUG_GUARD_BYTE 0xA5u
#define CL_DEBUG_FREED_BYTE 0xDDu
#define CL_FREE_LIST_MAGIC 0xC1FEEE1157A110C5ull
#define CL_FREE_LIST_SMALL_MAX_ALIGN 16u
#define CL_POOL_SLOT_FREE 0u
#define CL_POOL_SLOT_USED 1u

typedef struct cl_debug_header {
    struct cl_debug_header *next;
    uint64_t magic;
    void *raw;
    size_t raw_size;
    size_t raw_align;
    size_t user_size;
    size_t user_align;
} cl_debug_header;

typedef struct cl_free_list_node {
    size_t size;
    struct cl_free_list_node *next;
} cl_free_list_node;

typedef struct cl_free_list_small_node {
    size_t size;
    struct cl_free_list_small_node *next;
    size_t user_offset;
} cl_free_list_small_node;

typedef struct cl_free_list_header {
    uint64_t magic;
    size_t block_size;
    size_t user_offset;
    size_t user_size;
    size_t user_align;
} cl_free_list_header;

static const size_t cl_free_list_small_limits[CL_FREE_LIST_SMALL_BIN_COUNT] = {
    16u,
    32u,
    64u,
    96u
};

static size_t cl_max_align(void)
{
    /*
     * C99 does not have max_align_t. This union gives us a conservative
     * alignment suitable for ordinary malloc-backed objects on our target.
     */
    typedef union cl_align_probe {
        long double ld;
        long long ll;
        void *p;
        void (*fn)(void);
    } cl_align_probe;

    return sizeof(cl_align_probe);
}

bool cl_is_power_of_two(size_t value)
{
    return value != 0u && (value & (value - 1u)) == 0u;
}

bool cl_is_valid_align(size_t align)
{
    return cl_is_power_of_two(align);
}

bool cl_align_up_size(size_t value, size_t align, size_t *out)
{
    size_t mask;

    if (!out || !cl_is_valid_align(align)) {
        return false;
    }

    mask = align - 1u;
    /* Guard the addition before using the usual power-of-two rounding trick. */
    if (value > SIZE_MAX - mask) {
        return false;
    }

    *out = (value + mask) & ~mask;
    return true;
}

static bool cl_add_size(size_t a, size_t b, size_t *out)
{
    if (!out || a > SIZE_MAX - b) {
        return false;
    }

    *out = a + b;
    return true;
}

static size_t cl_normalize_align(size_t align)
{
    size_t min_align = sizeof(void *);

    if (align < min_align) {
        return min_align;
    }

    return align;
}

void *cl_alloc(cl_allocator *allocator, size_t size, size_t align)
{
    if (!allocator || !allocator->alloc || size == 0u || !cl_is_valid_align(align)) {
        return NULL;
    }

    return allocator->alloc(allocator->ctx, size, align);
}

void *cl_resize(
    cl_allocator *allocator,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    if (!allocator || !allocator->alloc || !cl_is_valid_align(align)) {
        return NULL;
    }

    if (!ptr) {
        return cl_alloc(allocator, new_size, align);
    }

    if (new_size == 0u) {
        cl_free(allocator, ptr, old_size, align);
        return NULL;
    }

    if (allocator->resize) {
        return allocator->resize(allocator->ctx, ptr, old_size, new_size, align);
    }

    /*
     * Fallback resize preserves realloc-style failure behavior: the original
     * block remains owned by the caller if allocating the replacement fails.
     */
    {
        void *next = cl_alloc(allocator, new_size, align);
        if (!next) {
            return NULL;
        }

        memcpy(next, ptr, old_size < new_size ? old_size : new_size);
        cl_free(allocator, ptr, old_size, align);
        return next;
    }
}

void cl_free(cl_allocator *allocator, void *ptr, size_t size, size_t align)
{
    if (!allocator || !allocator->free || !ptr) {
        return;
    }

    if (size == 0u || !cl_is_valid_align(align)) {
        return;
    }

    allocator->free(allocator->ctx, ptr, size, align);
}

static void *cl_system_alloc(void *ctx, size_t size, size_t align)
{
    void *ptr;
    size_t normalized_align;

    (void)ctx;

    normalized_align = cl_normalize_align(align);

    /*
     * malloc already satisfies fundamental alignment. Use posix_memalign only
     * for over-aligned requests so the common path stays as small as possible.
     */
    if (normalized_align <= cl_max_align()) {
        return malloc(size);
    }

    ptr = NULL;
    if (posix_memalign(&ptr, normalized_align, size) != 0) {
        return NULL;
    }

    return ptr;
}

static void *cl_system_resize(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    void *next;

    (void)ctx;

    if (align <= cl_max_align()) {
        return realloc(ptr, new_size);
    }

    next = cl_system_alloc(NULL, new_size, align);
    if (!next) {
        return NULL;
    }

    memcpy(next, ptr, old_size < new_size ? old_size : new_size);
    free(ptr);
    return next;
}

static void cl_system_free(void *ctx, void *ptr, size_t size, size_t align)
{
    (void)ctx;
    (void)size;
    (void)align;

    free(ptr);
}

cl_allocator cl_system_allocator(void)
{
    cl_allocator allocator;

    allocator.ctx = NULL;
    allocator.alloc = cl_system_alloc;
    allocator.resize = cl_system_resize;
    allocator.free = cl_system_free;
    return allocator;
}

void cl_arena_init(cl_arena *arena, void *buffer, size_t capacity)
{
    if (!arena) {
        return;
    }

    arena->base = (unsigned char *)buffer;
    arena->capacity = buffer ? capacity : 0u;
    arena->offset = 0u;
}

void cl_arena_reset(cl_arena *arena)
{
    if (arena) {
        arena->offset = 0u;
    }
}

size_t cl_arena_mark(const cl_arena *arena)
{
    return arena ? arena->offset : 0u;
}

bool cl_arena_restore(cl_arena *arena, size_t mark)
{
    if (!arena || mark > arena->offset) {
        return false;
    }

    arena->offset = mark;
    return true;
}

size_t cl_arena_used(const cl_arena *arena)
{
    return arena ? arena->offset : 0u;
}

size_t cl_arena_remaining(const cl_arena *arena)
{
    if (!arena || arena->offset > arena->capacity) {
        return 0u;
    }

    return arena->capacity - arena->offset;
}

static void *cl_arena_alloc(void *ctx, size_t size, size_t align)
{
    cl_arena *arena = (cl_arena *)ctx;
    uintptr_t base;
    uintptr_t current;
    uintptr_t aligned;
    size_t padding;
    size_t next_offset;

    if (!arena || !arena->base || size == 0u || !cl_is_valid_align(align)) {
        return NULL;
    }

    base = (uintptr_t)arena->base;
    if (arena->offset > arena->capacity || base > UINTPTR_MAX - arena->offset) {
        return NULL;
    }

    current = base + arena->offset;
    /*
     * The arena accepts any power-of-two alignment. Calculate padding from the
     * actual address, then account for it in offset-space with overflow checks.
     */
    aligned = (current + (uintptr_t)(align - 1u)) & ~(uintptr_t)(align - 1u);
    if (aligned < current) {
        return NULL;
    }

    padding = (size_t)(aligned - current);
    if (!cl_add_size(arena->offset, padding, &next_offset)) {
        return NULL;
    }

    if (!cl_add_size(next_offset, size, &next_offset) || next_offset > arena->capacity) {
        return NULL;
    }

    arena->offset = next_offset;
    return (void *)aligned;
}

static void *cl_arena_resize(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    cl_arena *arena = (cl_arena *)ctx;
    uintptr_t base;
    uintptr_t user;
    size_t start_offset;
    size_t old_end;
    size_t next_end;
    void *next;

    if (!arena || !ptr || !cl_is_valid_align(align)) {
        return NULL;
    }

    base = (uintptr_t)arena->base;
    user = (uintptr_t)ptr;
    if (user < base || user > base + arena->capacity) {
        return NULL;
    }

    start_offset = (size_t)(user - base);
    if (!cl_add_size(start_offset, old_size, &old_end)) {
        return NULL;
    }

    /* The most recent arena allocation can resize in place; older blocks move. */
    if (old_end == arena->offset && cl_add_size(start_offset, new_size, &next_end) &&
        next_end <= arena->capacity) {
        arena->offset = next_end;
        return ptr;
    }

    next = cl_arena_alloc(ctx, new_size, align);
    if (!next) {
        return NULL;
    }

    memcpy(next, ptr, old_size < new_size ? old_size : new_size);
    return next;
}

static void cl_arena_free(void *ctx, void *ptr, size_t size, size_t align)
{
    (void)ctx;
    (void)ptr;
    (void)size;
    (void)align;
}

cl_allocator cl_arena_allocator(cl_arena *arena)
{
    cl_allocator allocator;

    allocator.ctx = arena;
    allocator.alloc = cl_arena_alloc;
    allocator.resize = cl_arena_resize;
    allocator.free = cl_arena_free;
    return allocator;
}

static bool cl_pool_request_fits(const cl_pool *pool, size_t size, size_t align)
{
    return pool && size != 0u && size <= pool->block_size &&
           cl_is_valid_align(align) && align <= pool->block_align;
}

static bool cl_pool_slot_can_store_index(const cl_pool *pool)
{
    return pool && pool->block_stride >= sizeof(void *) + sizeof(size_t);
}

static void cl_pool_write_free_slot(cl_pool *pool, void *slot, void *next, size_t index)
{
    memcpy(slot, &next, sizeof(next));
    if (cl_pool_slot_can_store_index(pool)) {
        memcpy((unsigned char *)slot + sizeof(next), &index, sizeof(index));
    }
}

bool cl_pool_init(
    cl_pool *pool,
    void *buffer,
    size_t capacity,
    size_t block_size,
    size_t block_align)
{
    unsigned char *state;
    unsigned char *base;
    uintptr_t raw;
    uintptr_t aligned;
    uintptr_t end;
    size_t min_block_size;
    size_t stride;
    size_t count;
    size_t i;

    if (!pool) {
        return false;
    }

    pool->state = NULL;
    pool->base = NULL;
    pool->capacity = 0u;
    pool->block_size = 0u;
    pool->block_align = 0u;
    pool->block_stride = 0u;
    pool->block_count = 0u;
    pool->free_count = 0u;
    pool->free_list = NULL;
    pool->invalid_free_count = 0u;
    pool->mismatch_count = 0u;
    pool->double_free_count = 0u;

    if (!buffer || capacity == 0u || block_size == 0u ||
        !cl_is_valid_align(block_align)) {
        return false;
    }

    if (block_align < sizeof(void *)) {
        block_align = sizeof(void *);
    }

    min_block_size = block_size;
    if (min_block_size < sizeof(void *)) {
        min_block_size = sizeof(void *);
    }

    if (!cl_align_up_size(min_block_size, block_align, &stride)) {
        return false;
    }

    raw = (uintptr_t)buffer;
    if (capacity > UINTPTR_MAX - raw) {
        return false;
    }
    end = raw + capacity;

    count = capacity / stride;
    while (count != 0u) {
        uintptr_t after_state;
        size_t block_bytes;

        if (raw > UINTPTR_MAX - count) {
            return false;
        }

        after_state = raw + count;
        aligned = (after_state + (uintptr_t)(block_align - 1u)) &
                  ~(uintptr_t)(block_align - 1u);
        if (aligned < after_state || aligned > end) {
            count--;
            continue;
        }

        block_bytes = count * stride;
        if (block_bytes <= (size_t)(end - aligned)) {
            break;
        }

        count--;
    }

    if (count == 0u) {
        return false;
    }

    state = (unsigned char *)raw;
    base = (unsigned char *)aligned;
    pool->state = state;
    pool->base = base;
    pool->capacity = count * stride;
    pool->block_size = block_size;
    pool->block_align = block_align;
    pool->block_stride = stride;
    pool->block_count = count;
    pool->free_count = count;
    memset(pool->state, CL_POOL_SLOT_FREE, count);

    /*
     * Free slots store the next pointer in the slot itself. block_stride is
     * rounded up so every slot preserves the requested block alignment.
     */
    for (i = 0u; i < count; ++i) {
        unsigned char *slot = base + (i * stride);
        void *next = i + 1u < count ? (void *)(slot + stride) : NULL;
        cl_pool_write_free_slot(pool, slot, next, i);
    }
    pool->free_list = base;

    return true;
}

void cl_pool_reset(cl_pool *pool)
{
    size_t i;

    if (!pool || !pool->base || pool->block_count == 0u) {
        return;
    }

    for (i = 0u; i < pool->block_count; ++i) {
        unsigned char *slot = pool->base + (i * pool->block_stride);
        void *next = i + 1u < pool->block_count ?
                         (void *)(slot + pool->block_stride) :
                         NULL;
        cl_pool_write_free_slot(pool, slot, next, i);
    }
    pool->free_list = pool->base;
    pool->free_count = pool->block_count;
    memset(pool->state, CL_POOL_SLOT_FREE, pool->block_count);
}

size_t cl_pool_block_count(const cl_pool *pool)
{
    return pool ? pool->block_count : 0u;
}

size_t cl_pool_free_count(const cl_pool *pool)
{
    return pool ? pool->free_count : 0u;
}

size_t cl_pool_used_count(const cl_pool *pool)
{
    if (!pool || pool->free_count > pool->block_count) {
        return 0u;
    }

    return pool->block_count - pool->free_count;
}

size_t cl_pool_invalid_free_count(const cl_pool *pool)
{
    return pool ? pool->invalid_free_count : 0u;
}

size_t cl_pool_mismatch_count(const cl_pool *pool)
{
    return pool ? pool->mismatch_count : 0u;
}

size_t cl_pool_double_free_count(const cl_pool *pool)
{
    return pool ? pool->double_free_count : 0u;
}

static bool cl_pool_owns_slot(const cl_pool *pool, const void *ptr)
{
    uintptr_t base;
    uintptr_t slot;
    size_t offset;

    if (!pool || !pool->base || !ptr) {
        return false;
    }

    base = (uintptr_t)pool->base;
    slot = (uintptr_t)ptr;
    if (pool->capacity > UINTPTR_MAX - base || slot < base ||
        slot >= base + pool->capacity) {
        return false;
    }

    offset = (size_t)(slot - base);
    return (offset % pool->block_stride) == 0u;
}

static size_t cl_pool_slot_index(const cl_pool *pool, const void *ptr)
{
    uintptr_t base;
    uintptr_t slot;

    if (!pool || !pool->base || !ptr) {
        return 0u;
    }

    base = (uintptr_t)pool->base;
    slot = (uintptr_t)ptr;
    return (size_t)(slot - base) / pool->block_stride;
}

static bool cl_pool_slot_is_free(const cl_pool *pool, const void *ptr)
{
    size_t index;

    if (!pool || !pool->state || !cl_pool_owns_slot(pool, ptr)) {
        return false;
    }

    index = cl_pool_slot_index(pool, ptr);
    return index < pool->block_count && pool->state[index] == CL_POOL_SLOT_FREE;
}

static size_t cl_pool_alloc_slot_index(const cl_pool *pool, const void *slot)
{
    size_t index;

    if (cl_pool_slot_can_store_index(pool)) {
        memcpy(&index, (const unsigned char *)slot + sizeof(void *), sizeof(index));
        if (index < pool->block_count) {
            return index;
        }
    }

    return cl_pool_slot_index(pool, slot);
}

static void *cl_pool_alloc(void *ctx, size_t size, size_t align)
{
    cl_pool *pool = (cl_pool *)ctx;
    void *slot;
    size_t index;

    if (!cl_pool_request_fits(pool, size, align) || !pool->free_list) {
        return NULL;
    }

    slot = pool->free_list;
    memcpy(&pool->free_list, slot, sizeof(pool->free_list));
    index = cl_pool_alloc_slot_index(pool, slot);
    if (index < pool->block_count) {
        pool->state[index] = CL_POOL_SLOT_USED;
    }
    pool->free_count--;
    return slot;
}

static void *cl_pool_resize(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    cl_pool *pool = (cl_pool *)ctx;

    if (!pool || !ptr) {
        return NULL;
    }

    if (!cl_pool_owns_slot(pool, ptr)) {
        pool->invalid_free_count++;
        return NULL;
    }

    if (cl_pool_slot_is_free(pool, ptr)) {
        pool->double_free_count++;
        return NULL;
    }

    if (!cl_pool_request_fits(pool, old_size, align) ||
        !cl_pool_request_fits(pool, new_size, align)) {
        pool->mismatch_count++;
        return NULL;
    }

    return ptr;
}

static void cl_pool_free(void *ctx, void *ptr, size_t size, size_t align)
{
    cl_pool *pool = (cl_pool *)ctx;
    size_t index;

    if (!pool || !ptr) {
        return;
    }

    if (!cl_pool_owns_slot(pool, ptr)) {
        pool->invalid_free_count++;
        return;
    }

    if (!cl_pool_request_fits(pool, size, align)) {
        pool->mismatch_count++;
        return;
    }

    if (cl_pool_slot_is_free(pool, ptr)) {
        pool->double_free_count++;
        return;
    }

    index = cl_pool_slot_index(pool, ptr);
    cl_pool_write_free_slot(pool, ptr, pool->free_list, index);
    pool->free_list = ptr;
    if (index < pool->block_count) {
        pool->state[index] = CL_POOL_SLOT_FREE;
    }
    if (pool->free_count < pool->block_count) {
        pool->free_count++;
    }
}

cl_allocator cl_pool_allocator(cl_pool *pool)
{
    cl_allocator allocator;

    allocator.ctx = pool;
    allocator.alloc = cl_pool_alloc;
    allocator.resize = cl_pool_resize;
    allocator.free = cl_pool_free;
    return allocator;
}

static size_t cl_free_list_min_block_size(void)
{
    return sizeof(cl_free_list_node);
}

static size_t cl_free_list_small_bin_index(size_t size)
{
    size_t i;

    for (i = 0u; i < CL_FREE_LIST_SMALL_BIN_COUNT; ++i) {
        if (size <= cl_free_list_small_limits[i]) {
            return i;
        }
    }

    return CL_FREE_LIST_SMALL_BIN_COUNT;
}

static bool cl_free_list_is_small_request(size_t size, size_t align, size_t *out)
{
    size_t index;

    if (cl_normalize_align(align) > CL_FREE_LIST_SMALL_MAX_ALIGN) {
        return false;
    }

    index = cl_free_list_small_bin_index(size);
    if (index == CL_FREE_LIST_SMALL_BIN_COUNT) {
        return false;
    }

    if (out) {
        *out = index;
    }
    return true;
}

static void cl_free_list_clear_small_bins(cl_free_list *list)
{
    size_t i;

    if (!list) {
        return;
    }

    for (i = 0u; i < CL_FREE_LIST_SMALL_BIN_COUNT; ++i) {
        list->small_bins[i] = NULL;
    }
}

static bool cl_align_up_uintptr(uintptr_t value, size_t align, uintptr_t *out)
{
    uintptr_t mask;

    if (!out || !cl_is_valid_align(align) || align > (size_t)UINTPTR_MAX) {
        return false;
    }

    mask = (uintptr_t)(align - 1u);
    if (value > UINTPTR_MAX - mask) {
        return false;
    }

    *out = (value + mask) & ~mask;
    return true;
}

bool cl_free_list_init(cl_free_list *list, void *buffer, size_t capacity)
{
    uintptr_t raw;
    uintptr_t aligned;
    size_t padding;
    size_t min_block_size;
    cl_free_list_node *node;

    if (!list) {
        return false;
    }

    list->base = NULL;
    list->capacity = 0u;
    list->free_bytes = 0u;
    list->free_list = NULL;
    list->last_free = NULL;
    cl_free_list_clear_small_bins(list);
    list->invalid_free_count = 0u;
    list->mismatch_count = 0u;
    list->double_free_count = 0u;

    min_block_size = cl_free_list_min_block_size();
    if (!buffer || capacity < min_block_size) {
        return false;
    }

    raw = (uintptr_t)buffer;
    if (!cl_align_up_uintptr(raw, sizeof(void *), &aligned)) {
        return false;
    }

    padding = (size_t)(aligned - raw);
    if (padding > capacity || capacity - padding < min_block_size) {
        return false;
    }

    capacity -= padding;
    capacity -= capacity % sizeof(void *);

    list->base = (unsigned char *)aligned;
    list->capacity = capacity;
    list->free_bytes = capacity;

    node = (cl_free_list_node *)list->base;
    node->size = capacity;
    node->next = NULL;
    list->free_list = node;
    list->last_free = NULL;
    cl_free_list_clear_small_bins(list);

    return true;
}

void cl_free_list_reset(cl_free_list *list)
{
    cl_free_list_node *node;

    if (!list || !list->base || list->capacity < cl_free_list_min_block_size()) {
        return;
    }

    node = (cl_free_list_node *)list->base;
    node->size = list->capacity;
    node->next = NULL;
    list->free_list = node;
    list->last_free = NULL;
    cl_free_list_clear_small_bins(list);
    list->free_bytes = list->capacity;
}

size_t cl_free_list_capacity(const cl_free_list *list)
{
    return list ? list->capacity : 0u;
}

size_t cl_free_list_free_bytes(const cl_free_list *list)
{
    return list ? list->free_bytes : 0u;
}

size_t cl_free_list_used_bytes(const cl_free_list *list)
{
    if (!list || list->free_bytes > list->capacity) {
        return 0u;
    }

    return list->capacity - list->free_bytes;
}

size_t cl_free_list_invalid_free_count(const cl_free_list *list)
{
    return list ? list->invalid_free_count : 0u;
}

size_t cl_free_list_mismatch_count(const cl_free_list *list)
{
    return list ? list->mismatch_count : 0u;
}

size_t cl_free_list_double_free_count(const cl_free_list *list)
{
    return list ? list->double_free_count : 0u;
}

static bool cl_free_list_owns_ptr(const cl_free_list *list, const void *ptr)
{
    uintptr_t base;
    uintptr_t addr;

    if (!list || !list->base || !ptr) {
        return false;
    }

    base = (uintptr_t)list->base;
    addr = (uintptr_t)ptr;
    return list->capacity <= UINTPTR_MAX - base && addr >= base &&
           addr < base + list->capacity;
}

static bool cl_free_list_node_contains_ptr(const cl_free_list_node *node, uintptr_t addr)
{
    uintptr_t block;

    if (!node) {
        return false;
    }

    block = (uintptr_t)node;
    if (node->size > UINTPTR_MAX - block) {
        return false;
    }

    return addr >= block && addr < block + node->size;
}

static bool cl_free_list_ptr_is_free(const cl_free_list *list, const void *ptr)
{
    const cl_free_list_node *node;
    uintptr_t addr;
    size_t i;

    if (!list || !ptr) {
        return false;
    }

    addr = (uintptr_t)ptr;
    node = (const cl_free_list_node *)list->free_list;
    while (node) {
        if (cl_free_list_node_contains_ptr(node, addr)) {
            return true;
        }

        node = node->next;
    }

    for (i = 0u; i < CL_FREE_LIST_SMALL_BIN_COUNT; ++i) {
        node = (const cl_free_list_node *)list->small_bins[i];
        while (node) {
            if (cl_free_list_node_contains_ptr(node, addr)) {
                return true;
            }
            node = node->next;
        }
    }

    return false;
}

static void cl_free_list_insert_block(
    cl_free_list *list,
    unsigned char *block,
    size_t block_size)
{
    cl_free_list_node *node;
    cl_free_list_node *prev;
    cl_free_list_node *cur;
    cl_free_list_node *last;
    uintptr_t node_addr;

    if (!list || !block || block_size < cl_free_list_min_block_size()) {
        return;
    }

    node_addr = (uintptr_t)block;
    last = (cl_free_list_node *)list->last_free;
    if (last && (uintptr_t)last < node_addr &&
        (uintptr_t)last <= UINTPTR_MAX - last->size &&
        (uintptr_t)last + last->size == node_addr &&
        last->size <= SIZE_MAX - block_size) {
        cur = last->next;
        last->size += block_size;
        if (cur && (uintptr_t)last <= UINTPTR_MAX - last->size &&
            (uintptr_t)last + last->size == (uintptr_t)cur &&
            last->size <= SIZE_MAX - cur->size) {
            last->size += cur->size;
            last->next = cur->next;
        }
        return;
    }

    node = (cl_free_list_node *)block;
    node->size = block_size;
    node->next = NULL;

    prev = NULL;
    cur = (cl_free_list_node *)list->free_list;
    while (cur && (uintptr_t)cur < node_addr) {
        prev = cur;
        cur = cur->next;
    }

    node->next = cur;
    if (prev) {
        prev->next = node;
    } else {
        list->free_list = node;
    }

    if (cur && node_addr <= UINTPTR_MAX - node->size &&
        node_addr + node->size == (uintptr_t)cur) {
        node->size += cur->size;
        node->next = cur->next;
    }

    if (prev && (uintptr_t)prev <= UINTPTR_MAX - prev->size &&
        (uintptr_t)prev + prev->size == node_addr) {
        prev->size += node->size;
        prev->next = node->next;
        list->last_free = prev;
    } else {
        list->last_free = node;
    }
}

static bool cl_free_list_push_small_block(
    cl_free_list *list,
    unsigned char *block,
    size_t block_size,
    size_t user_offset,
    size_t user_size,
    size_t user_align)
{
    cl_free_list_small_node *node;
    size_t index;

    if (!list || !block ||
        !cl_free_list_is_small_request(user_size, user_align, &index) ||
        block_size < sizeof(*node) || user_offset > block_size ||
        user_size > block_size - user_offset) {
        return false;
    }

    node = (cl_free_list_small_node *)block;
    node->size = block_size;
    node->next = (cl_free_list_small_node *)list->small_bins[index];
    node->user_offset = user_offset;
    list->small_bins[index] = node;
    return true;
}

static void *cl_free_list_alloc_small(
    cl_free_list *list,
    size_t size,
    size_t align)
{
    size_t index;
    size_t i;

    if (!cl_free_list_is_small_request(size, align, &index)) {
        return NULL;
    }

    for (i = index; i < CL_FREE_LIST_SMALL_BIN_COUNT; ++i) {
        cl_free_list_small_node *prev = NULL;
        cl_free_list_small_node *node =
            (cl_free_list_small_node *)list->small_bins[i];

        while (node) {
            uintptr_t user_addr;
            uintptr_t block_start;
            uintptr_t block_end;

            block_start = (uintptr_t)node;
            if (node->size <= UINTPTR_MAX - block_start &&
                node->user_offset <= node->size &&
                size <= node->size - node->user_offset) {
                size_t block_size = node->size;
                size_t user_offset = node->user_offset;

                block_end = block_start + block_size;
                user_addr = block_start + user_offset;

                if (((user_addr & (uintptr_t)(align - 1u)) != 0u) ||
                    user_addr > UINTPTR_MAX - size || user_addr + size > block_end) {
                    prev = node;
                    node = node->next;
                    continue;
                }

                if (prev) {
                    prev->next = node->next;
                } else {
                    list->small_bins[i] = node->next;
                }

                {
                    cl_free_list_header *header =
                        (cl_free_list_header *)(user_addr - sizeof(*header));
                    header->magic = CL_FREE_LIST_MAGIC;
                    header->block_size = block_size;
                    header->user_offset = user_offset;
                    header->user_size = size;
                    header->user_align = align;
                }

                if (list->free_bytes >= block_size) {
                    list->free_bytes -= block_size;
                } else {
                    list->free_bytes = 0u;
                }

                return (void *)user_addr;
            }

            prev = node;
            node = node->next;
        }
    }

    return NULL;
}

static bool cl_free_list_flush_small_bins(cl_free_list *list)
{
    bool flushed = false;
    size_t i;

    if (!list) {
        return false;
    }

    for (i = 0u; i < CL_FREE_LIST_SMALL_BIN_COUNT; ++i) {
        cl_free_list_node *node = (cl_free_list_node *)list->small_bins[i];
        list->small_bins[i] = NULL;
        while (node) {
            cl_free_list_node *next = node->next;
            cl_free_list_insert_block(list, (unsigned char *)node, node->size);
            node = next;
            flushed = true;
        }
    }

    return flushed;
}

static bool cl_free_list_header_for_ptr(
    const cl_free_list *list,
    void *ptr,
    cl_free_list_header **out)
{
    cl_free_list_header *header;
    uintptr_t user;
    uintptr_t base;
    uintptr_t block_start;

    if (!out || !cl_free_list_owns_ptr(list, ptr)) {
        return false;
    }

    user = (uintptr_t)ptr;
    if (user < (uintptr_t)list->base + sizeof(*header)) {
        return false;
    }

    header = (cl_free_list_header *)((unsigned char *)ptr - sizeof(*header));
    if (header->magic != CL_FREE_LIST_MAGIC || header->user_offset < sizeof(*header)) {
        return false;
    }

    base = (uintptr_t)list->base;
    if (user < header->user_offset || user - header->user_offset < base) {
        return false;
    }

    block_start = user - header->user_offset;
    if (header->block_size > list->capacity ||
        block_start > base + list->capacity - header->block_size ||
        header->user_offset > header->block_size ||
        header->user_size > header->block_size - header->user_offset) {
        return false;
    }

    *out = header;
    return true;
}

static void *cl_free_list_alloc(void *ctx, size_t size, size_t align)
{
    cl_free_list *list = (cl_free_list *)ctx;
    cl_free_list_node *prev;
    cl_free_list_node *node;
    size_t effective_align;
    size_t min_block_size;
    bool small_request;
    bool flushed;

    if (!list || size == 0u || !cl_is_valid_align(align)) {
        return NULL;
    }

    effective_align = cl_normalize_align(align);
    if (effective_align > (size_t)UINTPTR_MAX) {
        return NULL;
    }
    min_block_size = cl_free_list_min_block_size();
    small_request = cl_free_list_is_small_request(size, align, NULL);
    flushed = false;

    if (small_request) {
        void *small = cl_free_list_alloc_small(list, size, align);
        if (small) {
            return small;
        }
    } else {
        flushed = cl_free_list_flush_small_bins(list);
    }

retry:
    prev = NULL;
    node = (cl_free_list_node *)list->free_list;
    while (node) {
        uintptr_t block_start = (uintptr_t)node;
        uintptr_t block_end;
        uintptr_t user_addr;
        uintptr_t tail_start;
        size_t prefix_size;
        size_t allocation_size;
        size_t tail_size;
        if (node->size > UINTPTR_MAX - block_start ||
            block_start > UINTPTR_MAX - sizeof(cl_free_list_header)) {
            return NULL;
        }
        block_end = block_start + node->size;

        if (!cl_align_up_uintptr(block_start + sizeof(cl_free_list_header),
                                 effective_align, &user_addr) ||
            user_addr > UINTPTR_MAX - size) {
            return NULL;
        }

        prefix_size = (size_t)(user_addr - sizeof(cl_free_list_header) - block_start);
        if (prefix_size != 0u && prefix_size < min_block_size) {
            prefix_size = 0u;
        }

        if (prefix_size != 0u) {
            user_addr = block_start + prefix_size + sizeof(cl_free_list_header);
            if (!cl_align_up_uintptr(user_addr, effective_align, &user_addr) ||
                user_addr > UINTPTR_MAX - size) {
                return NULL;
            }
        }

        if (user_addr + size > block_end) {
            prev = node;
            node = node->next;
            continue;
        }

        if (!cl_align_up_uintptr(user_addr + size, sizeof(void *), &tail_start)) {
            return NULL;
        }
        if (tail_start > block_end) {
            prev = node;
            node = node->next;
            continue;
        }

        tail_size = (size_t)(block_end - tail_start);
        if (tail_size < min_block_size) {
            tail_start = block_end;
            tail_size = 0u;
        }

        allocation_size = (size_t)(tail_start - (block_start + prefix_size));

        if (prefix_size != 0u) {
            node->size = prefix_size;
            if (tail_size != 0u) {
                cl_free_list_node *tail = (cl_free_list_node *)tail_start;
                tail->size = tail_size;
                tail->next = node->next;
                node->next = tail;
            }
        } else if (tail_size != 0u) {
            cl_free_list_node *tail = (cl_free_list_node *)tail_start;
            tail->size = tail_size;
            tail->next = node->next;
            if (prev) {
                prev->next = tail;
            } else {
                list->free_list = tail;
            }
        } else if (prev) {
            prev->next = node->next;
        } else {
            list->free_list = node->next;
        }
        list->last_free = NULL;

        {
            cl_free_list_header *header =
                (cl_free_list_header *)(user_addr - sizeof(*header));
            header->magic = CL_FREE_LIST_MAGIC;
            header->block_size = allocation_size;
            header->user_offset = (size_t)(user_addr - (block_start + prefix_size));
            header->user_size = size;
            header->user_align = align;
        }

        if (list->free_bytes >= allocation_size) {
            list->free_bytes -= allocation_size;
        } else {
            list->free_bytes = 0u;
        }

        return (void *)user_addr;
    }

    if (!flushed && cl_free_list_flush_small_bins(list)) {
        flushed = true;
        goto retry;
    }

    return NULL;
}

static void cl_free_list_free(void *ctx, void *ptr, size_t size, size_t align)
{
    cl_free_list *list = (cl_free_list *)ctx;
    cl_free_list_header *header;
    unsigned char *block;
    size_t block_size;

    if (!list || !ptr) {
        return;
    }

    if (size == 0u || !cl_is_valid_align(align)) {
        list->mismatch_count++;
        return;
    }

    if (!cl_free_list_header_for_ptr(list, ptr, &header)) {
        if (cl_free_list_owns_ptr(list, ptr) && cl_free_list_ptr_is_free(list, ptr)) {
            list->double_free_count++;
        } else {
            list->invalid_free_count++;
        }
        return;
    }

    if (header->user_size != size || header->user_align != align) {
        list->mismatch_count++;
        return;
    }

    block = (unsigned char *)ptr - header->user_offset;
    block_size = header->block_size;
    header->magic = 0u;

    if (!cl_free_list_push_small_block(
            list, block, block_size, header->user_offset, size, align)) {
        cl_free_list_insert_block(list, block, block_size);
    }
    if (list->free_bytes <= list->capacity - block_size) {
        list->free_bytes += block_size;
    } else {
        list->free_bytes = list->capacity;
    }
}

static void *cl_free_list_resize(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    cl_free_list *list = (cl_free_list *)ctx;
    cl_free_list_header *header;
    unsigned char *user;
    unsigned char *block;
    uintptr_t new_end;
    uintptr_t tail_start;
    size_t old_block_size;
    size_t new_block_size;
    size_t remainder;
    void *next;

    if (!ptr) {
        return cl_free_list_alloc(ctx, new_size, align);
    }

    if (new_size == 0u) {
        cl_free_list_free(ctx, ptr, old_size, align);
        return NULL;
    }

    if (!list) {
        return NULL;
    }

    if (old_size == 0u || !cl_is_valid_align(align)) {
        list->mismatch_count++;
        return NULL;
    }

    if (!cl_free_list_header_for_ptr(list, ptr, &header)) {
        if (cl_free_list_owns_ptr(list, ptr) && cl_free_list_ptr_is_free(list, ptr)) {
            list->double_free_count++;
        } else {
            list->invalid_free_count++;
        }
        return NULL;
    }

    if (header->user_size != old_size || header->user_align != align) {
        list->mismatch_count++;
        return NULL;
    }

    if (new_size <= old_size) {
        user = (unsigned char *)ptr;
        block = user - header->user_offset;
        old_block_size = header->block_size;

        if ((uintptr_t)user > UINTPTR_MAX - new_size ||
            !cl_align_up_uintptr((uintptr_t)user + new_size, sizeof(void *),
                                 &tail_start)) {
            return NULL;
        }

        new_end = tail_start;
        new_block_size = (size_t)(new_end - (uintptr_t)block);
        if (new_block_size > old_block_size) {
            return NULL;
        }
        remainder = old_block_size - new_block_size;

        header->user_size = new_size;
        if (remainder >= cl_free_list_min_block_size()) {
            header->block_size = new_block_size;
            cl_free_list_insert_block(list, (unsigned char *)new_end, remainder);
            if (list->free_bytes <= list->capacity - remainder) {
                list->free_bytes += remainder;
            } else {
                list->free_bytes = list->capacity;
            }
        }

        return ptr;
    }

    next = cl_free_list_alloc(ctx, new_size, align);
    if (!next) {
        return NULL;
    }

    memcpy(next, ptr, old_size);
    cl_free_list_free(ctx, ptr, old_size, align);
    return next;
}

cl_allocator cl_free_list_allocator(cl_free_list *list)
{
    cl_allocator allocator;

    allocator.ctx = list;
    allocator.alloc = cl_free_list_alloc;
    allocator.resize = cl_free_list_resize;
    allocator.free = cl_free_list_free;
    return allocator;
}

static void cl_debug_fill(unsigned char *ptr, size_t size, unsigned char byte)
{
    memset(ptr, byte, size);
}

static bool cl_debug_check_guard(const unsigned char *ptr)
{
    size_t i;

    for (i = 0u; i < CL_DEBUG_GUARD_SIZE; ++i) {
        if (ptr[i] != CL_DEBUG_GUARD_BYTE) {
            return false;
        }
    }

    return true;
}

static cl_debug_header *cl_debug_header_from_user(void *ptr)
{
    return (cl_debug_header *)((unsigned char *)ptr - CL_DEBUG_GUARD_SIZE -
                               sizeof(cl_debug_header));
}

static void *cl_debug_alloc(void *ctx, size_t size, size_t align)
{
    cl_debug_allocator *debug = (cl_debug_allocator *)ctx;
    cl_debug_header *header;
    unsigned char *raw;
    unsigned char *user;
    unsigned char *left_guard;
    unsigned char *right_guard;
    uintptr_t user_addr;
    size_t user_align;
    size_t raw_size;
    size_t raw_align;

    if (!debug || size == 0u || !cl_is_valid_align(align)) {
        return NULL;
    }

    user_align = align;
    if (user_align < cl_max_align()) {
        user_align = cl_max_align();
    }

    /*
     * Layout:
     * raw allocation | optional padding | header | left guard | user | right guard
     *
     * The header sits immediately before the left guard so we can recover it
     * from a user pointer while still returning the requested user alignment.
     */
    raw_align = cl_max_align();
    raw_size = sizeof(cl_debug_header);
    if (!cl_add_size(raw_size, CL_DEBUG_GUARD_SIZE, &raw_size) ||
        !cl_add_size(raw_size, user_align - 1u, &raw_size) ||
        !cl_add_size(raw_size, size, &raw_size) ||
        !cl_add_size(raw_size, CL_DEBUG_GUARD_SIZE, &raw_size)) {
        debug->failed_count++;
        return NULL;
    }

    raw = cl_alloc(&debug->backing, raw_size, raw_align);
    if (!raw) {
        debug->failed_count++;
        return NULL;
    }

    user_addr = (uintptr_t)(raw + sizeof(cl_debug_header) + CL_DEBUG_GUARD_SIZE);
    user_addr = (user_addr + (uintptr_t)(user_align - 1u)) & ~(uintptr_t)(user_align - 1u);
    user = (unsigned char *)user_addr;

    header = (cl_debug_header *)(user - CL_DEBUG_GUARD_SIZE - sizeof(cl_debug_header));
    left_guard = user - CL_DEBUG_GUARD_SIZE;
    right_guard = user + size;

    header->magic = CL_DEBUG_MAGIC;
    header->raw = raw;
    header->raw_size = raw_size;
    header->raw_align = raw_align;
    header->user_size = size;
    header->user_align = align;
    header->next = (cl_debug_header *)debug->blocks;
    debug->blocks = header;

    cl_debug_fill(left_guard, CL_DEBUG_GUARD_SIZE, CL_DEBUG_GUARD_BYTE);
    cl_debug_fill(right_guard, CL_DEBUG_GUARD_SIZE, CL_DEBUG_GUARD_BYTE);

    debug->live_bytes += size;
    if (debug->live_bytes > debug->peak_bytes) {
        debug->peak_bytes = debug->live_bytes;
    }
    debug->allocation_count++;

    return user;
}

static void cl_debug_free(void *ctx, void *ptr, size_t size, size_t align)
{
    cl_debug_allocator *debug = (cl_debug_allocator *)ctx;
    cl_debug_header *header;
    unsigned char *user;
    unsigned char *left_guard;
    unsigned char *right_guard;

    if (!debug || !ptr) {
        return;
    }

    header = cl_debug_header_from_user(ptr);
    user = (unsigned char *)ptr;

    /* A freed debug block remains quarantined, so its magic is still readable. */
    if (header->magic != CL_DEBUG_MAGIC) {
        if (header->magic == CL_DEBUG_FREED) {
            debug->double_free_count++;
            return;
        }
        debug->corruption_count++;
        return;
    }

    left_guard = user - CL_DEBUG_GUARD_SIZE;
    right_guard = user + header->user_size;

    if (!cl_debug_check_guard(left_guard) || !cl_debug_check_guard(right_guard)) {
        debug->corruption_count++;
    }

    if (size != header->user_size || align != header->user_align) {
        debug->mismatch_count++;
    }

    if (debug->live_bytes >= header->user_size) {
        debug->live_bytes -= header->user_size;
    } else {
        debug->live_bytes = 0u;
    }

    header->magic = CL_DEBUG_FREED;
    cl_debug_fill(user, header->user_size, CL_DEBUG_FREED_BYTE);
    debug->free_count++;

    /*
     * Keep debug blocks quarantined until release. That preserves metadata for
     * double-free detection without reading memory returned to the backing allocator.
     */
}

static void *cl_debug_resize(
    void *ctx,
    void *ptr,
    size_t old_size,
    size_t new_size,
    size_t align)
{
    cl_debug_allocator *debug = (cl_debug_allocator *)ctx;
    void *next;
    cl_debug_header *header;
    size_t copy_size;

    if (!ptr) {
        return cl_debug_alloc(ctx, new_size, align);
    }

    if (new_size == 0u) {
        cl_debug_free(ctx, ptr, old_size, align);
        return NULL;
    }

    header = cl_debug_header_from_user(ptr);
    if (header->magic != CL_DEBUG_MAGIC) {
        if (debug) {
            debug->corruption_count++;
        }
        return NULL;
    }

    /*
     * Allocate-copy-free keeps debug metadata simple and makes resize validate
     * the old block through the same guard and mismatch checks as cl_free.
     */
    next = cl_debug_alloc(ctx, new_size, align);
    if (!next) {
        return NULL;
    }

    copy_size = old_size < new_size ? old_size : new_size;
    if (copy_size > header->user_size) {
        copy_size = header->user_size;
    }
    memcpy(next, ptr, copy_size);
    cl_debug_free(ctx, ptr, old_size, align);
    return next;
}

void cl_debug_allocator_init(cl_debug_allocator *debug, cl_allocator backing)
{
    if (!debug) {
        return;
    }

    debug->backing = backing;
    debug->blocks = NULL;
    debug->live_bytes = 0u;
    debug->peak_bytes = 0u;
    debug->allocation_count = 0u;
    debug->free_count = 0u;
    debug->failed_count = 0u;
    debug->corruption_count = 0u;
    debug->mismatch_count = 0u;
    debug->double_free_count = 0u;
}

cl_allocator cl_debug_allocator_view(cl_debug_allocator *debug)
{
    cl_allocator allocator;

    allocator.ctx = debug;
    allocator.alloc = cl_debug_alloc;
    allocator.resize = cl_debug_resize;
    allocator.free = cl_debug_free;
    return allocator;
}

void cl_debug_allocator_release(cl_debug_allocator *debug)
{
    cl_debug_header *header;

    if (!debug) {
        return;
    }

    header = (cl_debug_header *)debug->blocks;
    while (header) {
        cl_debug_header *next = header->next;
        cl_free(&debug->backing, header->raw, header->raw_size, header->raw_align);
        header = next;
    }

    debug->blocks = NULL;
    debug->live_bytes = 0u;
}
