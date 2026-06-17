/*
 * cl_atomic.h
 * Purpose: Thin compiler-atomic wrappers for integer and pointer state.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_ATOMIC_H
#define CL_ATOMIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CL_ATOMIC_HAS_BUILTINS 1
#else
#define CL_ATOMIC_HAS_BUILTINS 0
#endif

typedef enum cl_memory_order {
    CL_MEMORY_ORDER_RELAXED = 0,
    CL_MEMORY_ORDER_CONSUME = 1,
    CL_MEMORY_ORDER_ACQUIRE = 2,
    CL_MEMORY_ORDER_RELEASE = 3,
    CL_MEMORY_ORDER_ACQ_REL = 4,
    CL_MEMORY_ORDER_SEQ_CST = 5
} cl_memory_order;

typedef struct cl_atomic_u32 {
    volatile uint32_t value;
} cl_atomic_u32;

typedef struct cl_atomic_u64 {
    volatile uint64_t value;
} cl_atomic_u64;

typedef struct cl_atomic_size {
    volatile size_t value;
} cl_atomic_size;

typedef struct cl_atomic_ptr {
    void * volatile value;
} cl_atomic_ptr;

void cl_atomic_u32_init(cl_atomic_u32 *atomic, uint32_t value);
uint32_t cl_atomic_u32_load(const cl_atomic_u32 *atomic, cl_memory_order order);
void cl_atomic_u32_store(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_exchange(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
bool cl_atomic_u32_compare_exchange(
    cl_atomic_u32 *atomic,
    uint32_t *expected,
    uint32_t desired,
    cl_memory_order success_order,
    cl_memory_order failure_order);
uint32_t cl_atomic_u32_fetch_add(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_sub(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_or(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_and(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_xor(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);

void cl_atomic_u64_init(cl_atomic_u64 *atomic, uint64_t value);
uint64_t cl_atomic_u64_load(const cl_atomic_u64 *atomic, cl_memory_order order);
void cl_atomic_u64_store(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
uint64_t cl_atomic_u64_exchange(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
bool cl_atomic_u64_compare_exchange(
    cl_atomic_u64 *atomic,
    uint64_t *expected,
    uint64_t desired,
    cl_memory_order success_order,
    cl_memory_order failure_order);
uint64_t cl_atomic_u64_fetch_add(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
uint64_t cl_atomic_u64_fetch_sub(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
uint64_t cl_atomic_u64_fetch_or(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
uint64_t cl_atomic_u64_fetch_and(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);
uint64_t cl_atomic_u64_fetch_xor(cl_atomic_u64 *atomic, uint64_t value, cl_memory_order order);

void cl_atomic_size_init(cl_atomic_size *atomic, size_t value);
size_t cl_atomic_size_load(const cl_atomic_size *atomic, cl_memory_order order);
void cl_atomic_size_store(cl_atomic_size *atomic, size_t value, cl_memory_order order);
size_t cl_atomic_size_exchange(cl_atomic_size *atomic, size_t value, cl_memory_order order);
bool cl_atomic_size_compare_exchange(
    cl_atomic_size *atomic,
    size_t *expected,
    size_t desired,
    cl_memory_order success_order,
    cl_memory_order failure_order);
size_t cl_atomic_size_fetch_add(cl_atomic_size *atomic, size_t value, cl_memory_order order);
size_t cl_atomic_size_fetch_sub(cl_atomic_size *atomic, size_t value, cl_memory_order order);
size_t cl_atomic_size_fetch_or(cl_atomic_size *atomic, size_t value, cl_memory_order order);
size_t cl_atomic_size_fetch_and(cl_atomic_size *atomic, size_t value, cl_memory_order order);
size_t cl_atomic_size_fetch_xor(cl_atomic_size *atomic, size_t value, cl_memory_order order);

void cl_atomic_ptr_init(cl_atomic_ptr *atomic, void *value);
void *cl_atomic_ptr_load(const cl_atomic_ptr *atomic, cl_memory_order order);
void cl_atomic_ptr_store(cl_atomic_ptr *atomic, void *value, cl_memory_order order);
void *cl_atomic_ptr_exchange(cl_atomic_ptr *atomic, void *value, cl_memory_order order);
bool cl_atomic_ptr_compare_exchange(
    cl_atomic_ptr *atomic,
    void **expected,
    void *desired,
    cl_memory_order success_order,
    cl_memory_order failure_order);

void cl_atomic_thread_fence(cl_memory_order order);
void cl_atomic_signal_fence(cl_memory_order order);

#ifdef __cplusplus
}
#endif

#endif
