<!--
docs/cl_file.md
Purpose: Whole-file helper documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_file

`cl_file` provides small POSIX whole-file helpers for reading byte files into a
caller-supplied allocator and writing complete byte buffers to disk.

## Example

```c
cl_allocator allocator = cl_system_allocator();
cl_file_data file;

if (cl_file_read_all("input.txt", &allocator, &file)) {
    /* file.data[file.size] is a convenience NUL byte. */
    (void)cl_file_write_all("copy.txt", file.data, file.size);
    cl_file_data_free(&file);
}
```

## Contracts

- `cl_file_read_all` initializes `out` before reading. On success, `out->data`
  owns `out->capacity` bytes through the allocator passed to the call.
- `out->size` is the number of file bytes read. `out->data[out->size]` is always
  a NUL byte for text convenience, but that terminator is not counted in `size`.
- `cl_file_data_free` releases owned storage and resets `data`, `size`, and
  `capacity`.
- `cl_file_write_all` creates or truncates the destination with mode `0666`
  filtered by the process umask.
- Null data is accepted by `cl_file_write_all` only when `size` is zero.

## Safety Properties

- Regular-file reads check the `stat` size before converting it to `size_t` and
  before adding terminator capacity.
- Short regular-file reads, allocation failures, open/read/write/close failures,
  and invalid arguments fail without returning partially owned data.
- Non-regular inputs are read in bounded chunks through `cl_buffer`, so pipes and
  similar streams do not require a known size.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and uses `open`,
`fstat`, `read`, `write`, and `close`. It retries interrupted reads and writes.
