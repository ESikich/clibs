/*
 * cl_endian.c
 * Purpose: Implement byte-order conversion and unaligned integer helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_endian.h"

#include <string.h>

cl_endian_order cl_endian_host(void)
{
    const uint16_t value = 1u;
    const unsigned char *bytes = (const unsigned char *)&value;

    return bytes[0] == 1u ? CL_ENDIAN_LITTLE : CL_ENDIAN_BIG;
}

int cl_endian_is_little(void)
{
    return cl_endian_host() == CL_ENDIAN_LITTLE;
}

int cl_endian_is_big(void)
{
    return cl_endian_host() == CL_ENDIAN_BIG;
}

uint16_t cl_bswap16(uint16_t value)
{
    return (uint16_t)(((uint16_t)(value & UINT16_C(0x00ff)) << 8) |
                      ((uint16_t)(value & UINT16_C(0xff00)) >> 8));
}

uint32_t cl_bswap32(uint32_t value)
{
    return ((value & UINT32_C(0x000000ff)) << 24) |
           ((value & UINT32_C(0x0000ff00)) << 8) |
           ((value & UINT32_C(0x00ff0000)) >> 8) |
           ((value & UINT32_C(0xff000000)) >> 24);
}

uint64_t cl_bswap64(uint64_t value)
{
    return ((value & UINT64_C(0x00000000000000ff)) << 56) |
           ((value & UINT64_C(0x000000000000ff00)) << 40) |
           ((value & UINT64_C(0x0000000000ff0000)) << 24) |
           ((value & UINT64_C(0x00000000ff000000)) << 8) |
           ((value & UINT64_C(0x000000ff00000000)) >> 8) |
           ((value & UINT64_C(0x0000ff0000000000)) >> 24) |
           ((value & UINT64_C(0x00ff000000000000)) >> 40) |
           ((value & UINT64_C(0xff00000000000000)) >> 56);
}

uint16_t cl_host_to_le16(uint16_t value)
{
    return cl_endian_is_little() ? value : cl_bswap16(value);
}

uint32_t cl_host_to_le32(uint32_t value)
{
    return cl_endian_is_little() ? value : cl_bswap32(value);
}

uint64_t cl_host_to_le64(uint64_t value)
{
    return cl_endian_is_little() ? value : cl_bswap64(value);
}

uint16_t cl_host_to_be16(uint16_t value)
{
    return cl_endian_is_big() ? value : cl_bswap16(value);
}

uint32_t cl_host_to_be32(uint32_t value)
{
    return cl_endian_is_big() ? value : cl_bswap32(value);
}

uint64_t cl_host_to_be64(uint64_t value)
{
    return cl_endian_is_big() ? value : cl_bswap64(value);
}

uint16_t cl_le16_to_host(uint16_t value)
{
    return cl_host_to_le16(value);
}

uint32_t cl_le32_to_host(uint32_t value)
{
    return cl_host_to_le32(value);
}

uint64_t cl_le64_to_host(uint64_t value)
{
    return cl_host_to_le64(value);
}

uint16_t cl_be16_to_host(uint16_t value)
{
    return cl_host_to_be16(value);
}

uint32_t cl_be32_to_host(uint32_t value)
{
    return cl_host_to_be32(value);
}

uint64_t cl_be64_to_host(uint64_t value)
{
    return cl_host_to_be64(value);
}

uint16_t cl_endian_load_le16(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return (uint16_t)((uint16_t)bytes[0] |
                      (uint16_t)((uint16_t)bytes[1] << 8));
}

uint32_t cl_endian_load_le32(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

uint64_t cl_endian_load_le64(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return (uint64_t)bytes[0] |
           ((uint64_t)bytes[1] << 8) |
           ((uint64_t)bytes[2] << 16) |
           ((uint64_t)bytes[3] << 24) |
           ((uint64_t)bytes[4] << 32) |
           ((uint64_t)bytes[5] << 40) |
           ((uint64_t)bytes[6] << 48) |
           ((uint64_t)bytes[7] << 56);
}

uint16_t cl_endian_load_be16(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return (uint16_t)((uint16_t)((uint16_t)bytes[0] << 8) |
                      (uint16_t)bytes[1]);
}

uint32_t cl_endian_load_be32(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

uint64_t cl_endian_load_be64(const void *src)
{
    const unsigned char *bytes = (const unsigned char *)src;

    return ((uint64_t)bytes[0] << 56) |
           ((uint64_t)bytes[1] << 48) |
           ((uint64_t)bytes[2] << 40) |
           ((uint64_t)bytes[3] << 32) |
           ((uint64_t)bytes[4] << 24) |
           ((uint64_t)bytes[5] << 16) |
           ((uint64_t)bytes[6] << 8) |
           (uint64_t)bytes[7];
}

uint16_t cl_endian_load_ne16(const void *src)
{
    uint16_t value;

    (void)memcpy(&value, src, sizeof(value));
    return value;
}

uint32_t cl_endian_load_ne32(const void *src)
{
    uint32_t value;

    (void)memcpy(&value, src, sizeof(value));
    return value;
}

uint64_t cl_endian_load_ne64(const void *src)
{
    uint64_t value;

    (void)memcpy(&value, src, sizeof(value));
    return value;
}

void cl_endian_store_le16(void *dst, uint16_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)(value & UINT16_C(0x00ff));
    bytes[1] = (unsigned char)((value >> 8) & UINT16_C(0x00ff));
}

void cl_endian_store_le32(void *dst, uint32_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)(value & UINT32_C(0x000000ff));
    bytes[1] = (unsigned char)((value >> 8) & UINT32_C(0x000000ff));
    bytes[2] = (unsigned char)((value >> 16) & UINT32_C(0x000000ff));
    bytes[3] = (unsigned char)((value >> 24) & UINT32_C(0x000000ff));
}

void cl_endian_store_le64(void *dst, uint64_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)(value & UINT64_C(0x00000000000000ff));
    bytes[1] = (unsigned char)((value >> 8) & UINT64_C(0x00000000000000ff));
    bytes[2] = (unsigned char)((value >> 16) & UINT64_C(0x00000000000000ff));
    bytes[3] = (unsigned char)((value >> 24) & UINT64_C(0x00000000000000ff));
    bytes[4] = (unsigned char)((value >> 32) & UINT64_C(0x00000000000000ff));
    bytes[5] = (unsigned char)((value >> 40) & UINT64_C(0x00000000000000ff));
    bytes[6] = (unsigned char)((value >> 48) & UINT64_C(0x00000000000000ff));
    bytes[7] = (unsigned char)((value >> 56) & UINT64_C(0x00000000000000ff));
}

void cl_endian_store_be16(void *dst, uint16_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)((value >> 8) & UINT16_C(0x00ff));
    bytes[1] = (unsigned char)(value & UINT16_C(0x00ff));
}

void cl_endian_store_be32(void *dst, uint32_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)((value >> 24) & UINT32_C(0x000000ff));
    bytes[1] = (unsigned char)((value >> 16) & UINT32_C(0x000000ff));
    bytes[2] = (unsigned char)((value >> 8) & UINT32_C(0x000000ff));
    bytes[3] = (unsigned char)(value & UINT32_C(0x000000ff));
}

void cl_endian_store_be64(void *dst, uint64_t value)
{
    unsigned char *bytes = (unsigned char *)dst;

    bytes[0] = (unsigned char)((value >> 56) & UINT64_C(0x00000000000000ff));
    bytes[1] = (unsigned char)((value >> 48) & UINT64_C(0x00000000000000ff));
    bytes[2] = (unsigned char)((value >> 40) & UINT64_C(0x00000000000000ff));
    bytes[3] = (unsigned char)((value >> 32) & UINT64_C(0x00000000000000ff));
    bytes[4] = (unsigned char)((value >> 24) & UINT64_C(0x00000000000000ff));
    bytes[5] = (unsigned char)((value >> 16) & UINT64_C(0x00000000000000ff));
    bytes[6] = (unsigned char)((value >> 8) & UINT64_C(0x00000000000000ff));
    bytes[7] = (unsigned char)(value & UINT64_C(0x00000000000000ff));
}

void cl_endian_store_ne16(void *dst, uint16_t value)
{
    (void)memcpy(dst, &value, sizeof(value));
}

void cl_endian_store_ne32(void *dst, uint32_t value)
{
    (void)memcpy(dst, &value, sizeof(value));
}

void cl_endian_store_ne64(void *dst, uint64_t value)
{
    (void)memcpy(dst, &value, sizeof(value));
}
