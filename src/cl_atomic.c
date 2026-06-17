/*
 * cl_atomic.c
 * Purpose: Implement thin compiler-atomic wrappers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_atomic.h"

#if !CL_ATOMIC_HAS_BUILTINS
#error "cl_atomic requires GCC- or Clang-compatible __atomic builtins"
#endif

static int cl_atomic_order(cl_memory_order order)
{
    switch (order) {
    case CL_MEMORY_ORDER_RELAXED:
        return __ATOMIC_RELAXED;
    case CL_MEMORY_ORDER_CONSUME:
        return __ATOMIC_CONSUME;
    case CL_MEMORY_ORDER_ACQUIRE:
        return __ATOMIC_ACQUIRE;
    case CL_MEMORY_ORDER_RELEASE:
        return __ATOMIC_RELEASE;
    case CL_MEMORY_ORDER_ACQ_REL:
        return __ATOMIC_ACQ_REL;
    case CL_MEMORY_ORDER_SEQ_CST:
        return __ATOMIC_SEQ_CST;
    }

    return __ATOMIC_SEQ_CST;
}

#define CL_ATOMIC_DEFINE_UNSIGNED(prefix, type)                                 \
    void prefix##_init(prefix *atomic, type value)                              \
    {                                                                          \
        if (atomic) {                                                           \
            atomic->value = value;                                              \
        }                                                                      \
    }                                                                          \
                                                                               \
    type prefix##_load(const prefix *atomic, cl_memory_order order)             \
    {                                                                          \
        return __atomic_load_n(&atomic->value, cl_atomic_order(order));         \
    }                                                                          \
                                                                               \
    void prefix##_store(prefix *atomic, type value, cl_memory_order order)      \
    {                                                                          \
        __atomic_store_n(&atomic->value, value, cl_atomic_order(order));        \
    }                                                                          \
                                                                               \
    type prefix##_exchange(prefix *atomic, type value, cl_memory_order order)   \
    {                                                                          \
        return __atomic_exchange_n(&atomic->value, value, cl_atomic_order(order)); \
    }                                                                          \
                                                                               \
    bool prefix##_compare_exchange(                                             \
        prefix *atomic,                                                        \
        type *expected,                                                        \
        type desired,                                                          \
        cl_memory_order success_order,                                         \
        cl_memory_order failure_order)                                         \
    {                                                                          \
        return __atomic_compare_exchange_n(                                    \
            &atomic->value,                                                    \
            expected,                                                          \
            desired,                                                           \
            false,                                                             \
            cl_atomic_order(success_order),                                    \
            cl_atomic_order(failure_order));                                   \
    }                                                                          \
                                                                               \
    type prefix##_fetch_add(prefix *atomic, type value, cl_memory_order order)  \
    {                                                                          \
        return __atomic_fetch_add(&atomic->value, value, cl_atomic_order(order)); \
    }                                                                          \
                                                                               \
    type prefix##_fetch_sub(prefix *atomic, type value, cl_memory_order order)  \
    {                                                                          \
        return __atomic_fetch_sub(&atomic->value, value, cl_atomic_order(order)); \
    }                                                                          \
                                                                               \
    type prefix##_fetch_or(prefix *atomic, type value, cl_memory_order order)   \
    {                                                                          \
        return __atomic_fetch_or(&atomic->value, value, cl_atomic_order(order)); \
    }                                                                          \
                                                                               \
    type prefix##_fetch_and(prefix *atomic, type value, cl_memory_order order)  \
    {                                                                          \
        return __atomic_fetch_and(&atomic->value, value, cl_atomic_order(order)); \
    }                                                                          \
                                                                               \
    type prefix##_fetch_xor(prefix *atomic, type value, cl_memory_order order)  \
    {                                                                          \
        return __atomic_fetch_xor(&atomic->value, value, cl_atomic_order(order)); \
    }

CL_ATOMIC_DEFINE_UNSIGNED(cl_atomic_u32, uint32_t)
CL_ATOMIC_DEFINE_UNSIGNED(cl_atomic_u64, uint64_t)
CL_ATOMIC_DEFINE_UNSIGNED(cl_atomic_size, size_t)

void cl_atomic_ptr_init(cl_atomic_ptr *atomic, void *value)
{
    if (atomic) {
        atomic->value = value;
    }
}

void *cl_atomic_ptr_load(const cl_atomic_ptr *atomic, cl_memory_order order)
{
    return __atomic_load_n(&atomic->value, cl_atomic_order(order));
}

void cl_atomic_ptr_store(cl_atomic_ptr *atomic, void *value, cl_memory_order order)
{
    __atomic_store_n(&atomic->value, value, cl_atomic_order(order));
}

void *cl_atomic_ptr_exchange(cl_atomic_ptr *atomic, void *value, cl_memory_order order)
{
    return __atomic_exchange_n(&atomic->value, value, cl_atomic_order(order));
}

bool cl_atomic_ptr_compare_exchange(
    cl_atomic_ptr *atomic,
    void **expected,
    void *desired,
    cl_memory_order success_order,
    cl_memory_order failure_order)
{
    return __atomic_compare_exchange_n(
        &atomic->value,
        expected,
        desired,
        false,
        cl_atomic_order(success_order),
        cl_atomic_order(failure_order));
}

void cl_atomic_thread_fence(cl_memory_order order)
{
    __atomic_thread_fence(cl_atomic_order(order));
}

void cl_atomic_signal_fence(cl_memory_order order)
{
    __atomic_signal_fence(cl_atomic_order(order));
}
