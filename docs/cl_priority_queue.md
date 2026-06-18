<!--
docs/cl_priority_queue.md
Purpose: Fixed-capacity priority queue library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_priority_queue

`cl_priority_queue` provides allocation-free priority queues backed by
caller-owned storage. Elements are copied into fixed-size slots and ordered by a
binary heap, so push and pop operations are O(log n) and peek is O(1).

## Example

```c
static int compare_int_max(const void *left, const void *right, void *user)
{
    const int *a = left;
    const int *b = right;

    (void)user;
    return (*a > *b) - (*a < *b);
}

int storage[8];
cl_priority_queue queue;
int value = 42;
int out = 0;

cl_priority_queue_init(
    &queue,
    storage,
    sizeof(storage),
    sizeof(storage[0]),
    compare_int_max,
    NULL);

if (cl_priority_queue_push(&queue, &value) &&
    cl_priority_queue_pop(&queue, &out)) {
    /* out == 42 */
}
```

## Comparator Contract

The comparator receives two element pointers and the user pointer passed to
`cl_priority_queue_init`. It must return a positive value when the left element
has higher priority than the right element, a negative value when it has lower
priority, and zero when the elements are equivalent for heap ordering.

Reverse the comparator to build a min-priority queue.

## Contracts

- The storage must remain alive until the queue is no longer used.
- `element_size` must match the object size passed to
  `cl_priority_queue_push` and `cl_priority_queue_pop`.
- Capacity is derived as `storage_size / element_size`; trailing bytes that do
  not fit a complete element are ignored.
- `cl_priority_queue_push` copies exactly one element into the queue.
- `cl_priority_queue_pop` copies the highest-priority element out when `out` is
  non-null; passing null discards it.
- `cl_priority_queue_peek` returns a pointer into queue storage that remains
  valid until the next mutating priority queue operation.
- Mutating an element through a retained storage pointer can break the heap
  invariant; remove and reinsert changed elements instead.

## Safety Properties

- The implementation performs no allocation and cannot fail due to allocator
  state.
- Initialization rejects null storage, zero-sized storage, zero-sized elements,
  null comparators, and storage too small for one complete element by creating a
  zero-capacity queue.
- Queue capacity is computed from the actual storage byte size, which keeps slot
  address calculations within the caller-provided storage.
- Push rejects full queues without mutating existing elements.
- Pop rejects empty queues without reading storage.

## Portability

The implementation targets POSIX.1-2008 compatible C99. It uses only standard C
byte copying and pointer arithmetic within caller-provided objects.
