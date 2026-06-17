/*
 * cl_alloc.c
 * Purpose: Allocator implementations for system, arena, and debug allocation.
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

typedef struct cl_debug_header {
    struct cl_debug_header *next;
    uint64_t magic;
    void *raw;
    size_t raw_size;
    size_t raw_align;
    size_t user_size;
    size_t user_align;
} cl_debug_header;

static size_t cl_max_align(void)
{
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
