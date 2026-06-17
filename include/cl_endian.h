/*
 * cl_endian.h
 * Purpose: Byte-order conversion and unaligned integer load/store helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_ENDIAN_H
#define CL_ENDIAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cl_endian_order {
    CL_ENDIAN_LITTLE = 0,
    CL_ENDIAN_BIG = 1
} cl_endian_order;

cl_endian_order cl_endian_host(void);
int cl_endian_is_little(void);
int cl_endian_is_big(void);

uint16_t cl_bswap16(uint16_t value);
uint32_t cl_bswap32(uint32_t value);
uint64_t cl_bswap64(uint64_t value);

uint16_t cl_host_to_le16(uint16_t value);
uint32_t cl_host_to_le32(uint32_t value);
uint64_t cl_host_to_le64(uint64_t value);
uint16_t cl_host_to_be16(uint16_t value);
uint32_t cl_host_to_be32(uint32_t value);
uint64_t cl_host_to_be64(uint64_t value);

uint16_t cl_le16_to_host(uint16_t value);
uint32_t cl_le32_to_host(uint32_t value);
uint64_t cl_le64_to_host(uint64_t value);
uint16_t cl_be16_to_host(uint16_t value);
uint32_t cl_be32_to_host(uint32_t value);
uint64_t cl_be64_to_host(uint64_t value);

uint16_t cl_endian_load_le16(const void *src);
uint32_t cl_endian_load_le32(const void *src);
uint64_t cl_endian_load_le64(const void *src);
uint16_t cl_endian_load_be16(const void *src);
uint32_t cl_endian_load_be32(const void *src);
uint64_t cl_endian_load_be64(const void *src);
uint16_t cl_endian_load_ne16(const void *src);
uint32_t cl_endian_load_ne32(const void *src);
uint64_t cl_endian_load_ne64(const void *src);

void cl_endian_store_le16(void *dst, uint16_t value);
void cl_endian_store_le32(void *dst, uint32_t value);
void cl_endian_store_le64(void *dst, uint64_t value);
void cl_endian_store_be16(void *dst, uint16_t value);
void cl_endian_store_be32(void *dst, uint32_t value);
void cl_endian_store_be64(void *dst, uint64_t value);
void cl_endian_store_ne16(void *dst, uint16_t value);
void cl_endian_store_ne32(void *dst, uint32_t value);
void cl_endian_store_ne64(void *dst, uint64_t value);

#ifdef __cplusplus
}
#endif

#endif
