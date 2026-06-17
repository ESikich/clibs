<!--
docs/cl_endian.md
Purpose: Endian helper library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_endian

`cl_endian` provides allocation-free byte-order conversion and unaligned integer
load/store helpers for 16-bit, 32-bit, and 64-bit unsigned integers.

## Interface

```c
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
```

## Contracts

The big-endian and little-endian load/store helpers interpret bytes in the named
order regardless of the host CPU. They operate on `void *` byte spans and do not
require alignment. Callers must still provide enough readable or writable bytes
for the selected integer width.

The native-endian load/store helpers copy the object representation with
`memcpy`, so they are safe for unaligned addresses. They are intended for local
binary layouts where host byte order is part of the contract. Use explicit
little-endian or big-endian helpers for portable file and wire formats.

The conversion helpers are value conversions. `cl_host_to_le16` and
`cl_le16_to_host` are inverses of each other, as are the matching 32-bit,
64-bit, and big-endian variants.

## Portability

The implementation uses C99 fixed-width unsigned integer types and POSIX-safe
library calls only. It does not depend on platform endian macros, unaligned
pointer casts, compiler byte-swap intrinsics, or non-standard headers.
