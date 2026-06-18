<!--
docs/cl_queue.md
Purpose: Fixed-capacity FIFO queue library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_queue

`cl_queue` provides allocation-free FIFO queues backed by caller-owned storage.
Elements are copied into fixed-size slots, and the queue uses ring indexing so
push and pop operations are O(1).

## Example

```c
int storage[8];
cl_queue queue;
int value = 42;
int out = 0;

cl_queue_init(&queue, storage, sizeof(storage), sizeof(storage[0]));

if (cl_queue_push(&queue, &value) && cl_queue_pop(&queue, &out)) {
    /* out == 42 */
}
```

## Contracts

- The storage must remain alive until the queue is no longer used.
- `element_size` must match the object size passed to `cl_queue_push` and
  `cl_queue_pop`.
- Capacity is derived as `storage_size / element_size`; trailing bytes that do
  not fit a complete element are ignored.
- `cl_queue_push` copies exactly one element into the queue.
- `cl_queue_pop` copies the oldest element out when `out` is non-null; passing
  null discards the oldest element.
- `cl_queue_front` and `cl_queue_front_mut` return pointers into queue storage
  that remain valid until the next mutating queue operation.

## Safety Properties

- The implementation performs no allocation and cannot fail due to allocator
  state.
- Initialization rejects null storage, zero-sized storage, zero-sized elements,
  and storage too small for one complete element by creating a zero-capacity
  queue.
- Queue capacity is computed from the actual storage byte size, which keeps slot
  address calculations within the caller-provided storage.
- Push rejects full queues without mutating existing elements.
- Pop rejects empty queues without reading storage.

## Portability

The implementation targets POSIX.1-2008 compatible C99. It uses only standard C
byte copying and pointer arithmetic within caller-provided objects.
