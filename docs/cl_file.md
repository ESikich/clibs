<!--
docs/cl_file.md
Purpose: Whole-file and streaming file helper documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_file

`cl_file` provides small POSIX helpers for whole-file reads/writes and explicit
streaming reads/writes.

## Example

```c
cl_allocator allocator = cl_system_allocator();
cl_file_data file;
cl_file input;
cl_file output;
unsigned char chunk[256];
size_t nread;

if (cl_file_read_all("input.txt", &allocator, &file)) {
    /* file.data[file.size] is a convenience NUL byte. */
    (void)cl_file_write_all("copy.txt", file.data, file.size);
    cl_file_data_free(&file);
}

if (cl_file_open_read("input.txt", &input) &&
    cl_file_open_write("copy-stream.txt", &output)) {
    while (cl_file_read(&input, chunk, sizeof(chunk), &nread) && nread != 0u) {
        if (!cl_file_write(&output, chunk, nread)) {
            break;
        }
    }
    (void)cl_file_close(&output);
    (void)cl_file_close(&input);
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
- `cl_file_open_read`, `cl_file_open_write`, and `cl_file_open_append`
  initialize the stream handle before opening. Write mode truncates, append mode
  preserves existing data and writes at end of file.
- `cl_file_read` retries interrupted reads, reports the number of bytes read,
  and returns success with `*nread == 0` at EOF.
- `cl_file_write` retries interrupted writes and succeeds only after the full
  supplied buffer has been written.
- `cl_file_close` closes an open stream and invalidates the handle.

## Safety Properties

- Regular-file reads check the `stat` size before converting it to `size_t` and
  before adding terminator capacity.
- Short regular-file reads, allocation failures, open/read/write/close failures,
  and invalid arguments fail without returning partially owned whole-file data.
- Non-regular inputs are read in bounded chunks through `cl_buffer`, so pipes and
  similar streams do not require a known size.
- Streaming helpers keep ownership explicit: callers own stream handles, caller
  buffers, and any retry policy above one call at a time.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and uses `open`,
`fstat`, `read`, `write`, and `close`. It retries interrupted reads and writes.
