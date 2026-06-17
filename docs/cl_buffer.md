<!--
docs/cl_buffer.md
Purpose: Byte buffer and ring buffer library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_buffer

`cl_buffer` provides an owned growable byte buffer backed by `cl_allocator` and a
caller-owned ring buffer for bounded FIFO byte streams.

## Growable Buffers

```c
cl_allocator allocator = cl_system_allocator();
cl_buffer bytes;

cl_buffer_init(&bytes, &allocator);
if (cl_buffer_append(&bytes, "abc", 3u)) {
    (void)cl_buffer_append_byte(&bytes, (unsigned char)'\n');
}
cl_buffer_free(&bytes);
```

## Ring Buffers

```c
unsigned char storage[128];
cl_ring_buffer ring;
unsigned char out[16];

cl_ring_buffer_init(&ring, storage, sizeof(storage));
(void)cl_ring_buffer_write(&ring, "hello", 5u);
(void)cl_ring_buffer_read(&ring, out, sizeof(out));
```

## Contracts

- The allocator passed to `cl_buffer_init` must outlive the buffer.
- `cl_buffer_free` releases backing storage and resets `data`, `size`, and
  `capacity`.
- `cl_buffer_clear` drops contents without releasing capacity.
- `cl_buffer_resize` grows as needed and zero-initializes newly exposed bytes.
- `cl_buffer_append` accepts a null data pointer only when the appended size is
  zero.
- `cl_ring_buffer_init` does not allocate; the storage must outlive the ring.
- Ring buffer bulk read/write operations transfer as many bytes as fit or are
  available and return the count transferred.

## Safety Properties

- Buffer append checks `size + append_size` for overflow before growing.
- Failed buffer growth leaves the original data pointer, size, and capacity
  unchanged.
- Ring buffers keep one explicit `size`, so all provided storage bytes are
  usable and full/empty states are unambiguous.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and uses only standard C
library byte operations plus the project allocator interface.
