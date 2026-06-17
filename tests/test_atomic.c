/*
 * test_atomic.c
 * Purpose: Safety and behavior tests for cl_atomic.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_atomic.h"
#include "cl_test.h"

#include <stdint.h>

static int test_u32_operations(void)
{
    cl_atomic_u32 value;
    uint32_t expected;

    cl_atomic_u32_init(&value, UINT32_C(10));
    CL_TEST_CHECK(cl_atomic_u32_load(&value, CL_MEMORY_ORDER_RELAXED) ==
                  UINT32_C(10));

    cl_atomic_u32_store(&value, UINT32_C(11), CL_MEMORY_ORDER_RELEASE);
    CL_TEST_CHECK(cl_atomic_u32_exchange(
                      &value,
                      UINT32_C(12),
                      CL_MEMORY_ORDER_ACQ_REL) == UINT32_C(11));
    CL_TEST_CHECK(cl_atomic_u32_load(&value, CL_MEMORY_ORDER_ACQUIRE) ==
                  UINT32_C(12));

    expected = UINT32_C(7);
    CL_TEST_CHECK(!cl_atomic_u32_compare_exchange(
        &value,
        &expected,
        UINT32_C(20),
        CL_MEMORY_ORDER_SEQ_CST,
        CL_MEMORY_ORDER_RELAXED));
    CL_TEST_CHECK(expected == UINT32_C(12));

    CL_TEST_CHECK(cl_atomic_u32_compare_exchange(
        &value,
        &expected,
        UINT32_C(20),
        CL_MEMORY_ORDER_SEQ_CST,
        CL_MEMORY_ORDER_RELAXED));
    CL_TEST_CHECK(cl_atomic_u32_fetch_add(
                      &value,
                      UINT32_C(2),
                      CL_MEMORY_ORDER_SEQ_CST) == UINT32_C(20));
    CL_TEST_CHECK(cl_atomic_u32_fetch_sub(
                      &value,
                      UINT32_C(5),
                      CL_MEMORY_ORDER_SEQ_CST) == UINT32_C(22));
    CL_TEST_CHECK(cl_atomic_u32_load(&value, CL_MEMORY_ORDER_SEQ_CST) ==
                  UINT32_C(17));
    return 0;
}

static int test_u64_bitwise_operations(void)
{
    cl_atomic_u64 value;

    cl_atomic_u64_init(&value, UINT64_C(0x0f0f));
    CL_TEST_CHECK(cl_atomic_u64_fetch_or(
                      &value,
                      UINT64_C(0xf000),
                      CL_MEMORY_ORDER_SEQ_CST) == UINT64_C(0x0f0f));
    CL_TEST_CHECK(cl_atomic_u64_fetch_and(
                      &value,
                      UINT64_C(0xff00),
                      CL_MEMORY_ORDER_SEQ_CST) == UINT64_C(0xff0f));
    CL_TEST_CHECK(cl_atomic_u64_fetch_xor(
                      &value,
                      UINT64_C(0x0f00),
                      CL_MEMORY_ORDER_SEQ_CST) == UINT64_C(0xff00));
    CL_TEST_CHECK(cl_atomic_u64_load(&value, CL_MEMORY_ORDER_SEQ_CST) ==
                  UINT64_C(0xf000));
    return 0;
}

static int test_size_operations(void)
{
    cl_atomic_size value;
    size_t expected = 3u;

    cl_atomic_size_init(&value, 3u);
    CL_TEST_CHECK(cl_atomic_size_compare_exchange(
        &value,
        &expected,
        8u,
        CL_MEMORY_ORDER_SEQ_CST,
        CL_MEMORY_ORDER_SEQ_CST));
    CL_TEST_CHECK(cl_atomic_size_fetch_add(
                      &value,
                      4u,
                      CL_MEMORY_ORDER_SEQ_CST) == 8u);
    CL_TEST_CHECK(cl_atomic_size_fetch_sub(
                      &value,
                      2u,
                      CL_MEMORY_ORDER_SEQ_CST) == 12u);
    CL_TEST_CHECK(cl_atomic_size_load(&value, CL_MEMORY_ORDER_SEQ_CST) == 10u);
    return 0;
}

static int test_pointer_operations(void)
{
    cl_atomic_ptr value;
    int first = 1;
    int second = 2;
    int third = 3;
    void *expected;

    cl_atomic_ptr_init(&value, &first);
    CL_TEST_CHECK(cl_atomic_ptr_load(&value, CL_MEMORY_ORDER_RELAXED) == &first);
    CL_TEST_CHECK(cl_atomic_ptr_exchange(
                      &value,
                      &second,
                      CL_MEMORY_ORDER_SEQ_CST) == &first);

    expected = &first;
    CL_TEST_CHECK(!cl_atomic_ptr_compare_exchange(
        &value,
        &expected,
        &third,
        CL_MEMORY_ORDER_SEQ_CST,
        CL_MEMORY_ORDER_SEQ_CST));
    CL_TEST_CHECK(expected == &second);

    CL_TEST_CHECK(cl_atomic_ptr_compare_exchange(
        &value,
        &expected,
        &third,
        CL_MEMORY_ORDER_SEQ_CST,
        CL_MEMORY_ORDER_SEQ_CST));
    CL_TEST_CHECK(cl_atomic_ptr_load(&value, CL_MEMORY_ORDER_SEQ_CST) == &third);
    return 0;
}

static int test_fences_are_callable(void)
{
    cl_atomic_signal_fence(CL_MEMORY_ORDER_ACQUIRE);
    cl_atomic_thread_fence(CL_MEMORY_ORDER_RELEASE);
    cl_atomic_thread_fence(CL_MEMORY_ORDER_SEQ_CST);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_u32_operations),
        CL_TEST_CASE(test_u64_bitwise_operations),
        CL_TEST_CASE(test_size_operations),
        CL_TEST_CASE(test_pointer_operations),
        CL_TEST_CASE(test_fences_are_callable)
    };

    return cl_test_run_all("cl_atomic", cases, sizeof(cases) / sizeof(cases[0]));
}
