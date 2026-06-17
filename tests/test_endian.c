/*
 * test_endian.c
 * Purpose: Safety and behavior tests for cl_endian.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_endian.h"
#include "cl_test.h"

#include <string.h>

static int test_host_order_detection_is_consistent(void)
{
    CL_TEST_CHECK(cl_endian_host() == CL_ENDIAN_LITTLE ||
                  cl_endian_host() == CL_ENDIAN_BIG);
    CL_TEST_CHECK(cl_endian_is_little() != cl_endian_is_big());
    CL_TEST_CHECK((cl_endian_host() == CL_ENDIAN_LITTLE) ==
                  (cl_endian_is_little() != 0));
    CL_TEST_CHECK((cl_endian_host() == CL_ENDIAN_BIG) ==
                  (cl_endian_is_big() != 0));
    return 0;
}

static int test_byte_swaps_and_round_trips(void)
{
    CL_TEST_CHECK(cl_bswap16(UINT16_C(0x1234)) == UINT16_C(0x3412));
    CL_TEST_CHECK(cl_bswap32(UINT32_C(0x12345678)) == UINT32_C(0x78563412));
    CL_TEST_CHECK(
        cl_bswap64(UINT64_C(0x0123456789abcdef)) ==
        UINT64_C(0xefcdab8967452301));

    CL_TEST_CHECK(cl_le16_to_host(cl_host_to_le16(UINT16_C(0x1234))) ==
                  UINT16_C(0x1234));
    CL_TEST_CHECK(cl_le32_to_host(cl_host_to_le32(UINT32_C(0x12345678))) ==
                  UINT32_C(0x12345678));
    CL_TEST_CHECK(
        cl_le64_to_host(cl_host_to_le64(UINT64_C(0x0123456789abcdef))) ==
        UINT64_C(0x0123456789abcdef));
    CL_TEST_CHECK(cl_be16_to_host(cl_host_to_be16(UINT16_C(0x1234))) ==
                  UINT16_C(0x1234));
    CL_TEST_CHECK(cl_be32_to_host(cl_host_to_be32(UINT32_C(0x12345678))) ==
                  UINT32_C(0x12345678));
    CL_TEST_CHECK(
        cl_be64_to_host(cl_host_to_be64(UINT64_C(0x0123456789abcdef))) ==
        UINT64_C(0x0123456789abcdef));
    return 0;
}

static int test_little_endian_loads_and_stores(void)
{
    unsigned char bytes[10];

    (void)memset(bytes, 0, sizeof(bytes));
    cl_endian_store_le16(bytes + 1u, UINT16_C(0x1234));
    CL_TEST_CHECK(bytes[1] == 0x34u);
    CL_TEST_CHECK(bytes[2] == 0x12u);
    CL_TEST_CHECK(cl_endian_load_le16(bytes + 1u) == UINT16_C(0x1234));

    cl_endian_store_le32(bytes + 1u, UINT32_C(0x12345678));
    CL_TEST_CHECK(bytes[1] == 0x78u);
    CL_TEST_CHECK(bytes[2] == 0x56u);
    CL_TEST_CHECK(bytes[3] == 0x34u);
    CL_TEST_CHECK(bytes[4] == 0x12u);
    CL_TEST_CHECK(cl_endian_load_le32(bytes + 1u) == UINT32_C(0x12345678));

    cl_endian_store_le64(bytes + 1u, UINT64_C(0x0123456789abcdef));
    CL_TEST_CHECK(bytes[1] == 0xefu);
    CL_TEST_CHECK(bytes[8] == 0x01u);
    CL_TEST_CHECK(
        cl_endian_load_le64(bytes + 1u) == UINT64_C(0x0123456789abcdef));
    return 0;
}

static int test_big_endian_loads_and_stores(void)
{
    unsigned char bytes[10];

    (void)memset(bytes, 0, sizeof(bytes));
    cl_endian_store_be16(bytes + 1u, UINT16_C(0x1234));
    CL_TEST_CHECK(bytes[1] == 0x12u);
    CL_TEST_CHECK(bytes[2] == 0x34u);
    CL_TEST_CHECK(cl_endian_load_be16(bytes + 1u) == UINT16_C(0x1234));

    cl_endian_store_be32(bytes + 1u, UINT32_C(0x12345678));
    CL_TEST_CHECK(bytes[1] == 0x12u);
    CL_TEST_CHECK(bytes[2] == 0x34u);
    CL_TEST_CHECK(bytes[3] == 0x56u);
    CL_TEST_CHECK(bytes[4] == 0x78u);
    CL_TEST_CHECK(cl_endian_load_be32(bytes + 1u) == UINT32_C(0x12345678));

    cl_endian_store_be64(bytes + 1u, UINT64_C(0x0123456789abcdef));
    CL_TEST_CHECK(bytes[1] == 0x01u);
    CL_TEST_CHECK(bytes[8] == 0xefu);
    CL_TEST_CHECK(
        cl_endian_load_be64(bytes + 1u) == UINT64_C(0x0123456789abcdef));
    return 0;
}

static int test_native_unaligned_loads_and_stores(void)
{
    unsigned char bytes[10];
    uint64_t expected64 = UINT64_C(0x0123456789abcdef);
    uint32_t expected32 = UINT32_C(0x12345678);
    uint16_t expected16 = UINT16_C(0x1234);

    (void)memset(bytes, 0, sizeof(bytes));
    cl_endian_store_ne16(bytes + 1u, expected16);
    CL_TEST_CHECK(cl_endian_load_ne16(bytes + 1u) == expected16);

    cl_endian_store_ne32(bytes + 1u, expected32);
    CL_TEST_CHECK(cl_endian_load_ne32(bytes + 1u) == expected32);

    cl_endian_store_ne64(bytes + 1u, expected64);
    CL_TEST_CHECK(cl_endian_load_ne64(bytes + 1u) == expected64);
    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_host_order_detection_is_consistent),
        CL_TEST_CASE(test_byte_swaps_and_round_trips),
        CL_TEST_CASE(test_little_endian_loads_and_stores),
        CL_TEST_CASE(test_big_endian_loads_and_stores),
        CL_TEST_CASE(test_native_unaligned_loads_and_stores)
    };

    return cl_test_run_all("cl_endian", cases, sizeof(cases) / sizeof(cases[0]));
}
