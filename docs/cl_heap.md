<!--
docs/cl_heap.md
Purpose: Binary heap algorithm documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_heap

`cl_heap` provides allocation-free binary heap algorithms over caller-owned
arrays. Unlike `cl_priority_queue`, it does not own size or capacity state: the
caller manages the array length and passes the active element count to each
operation.

## Example

```c
static int compare_int_max(const void *left, const void *right, void *user)
{
    const int *a = left;
    const int *b = right;

    (void)user;
    return (*a > *b) - (*a < *b);
}

int values[] = {4, 1, 8, 3};
size_t count = sizeof(values) / sizeof(values[0]);

if (cl_heap_make(values, count, sizeof(values[0]), compare_int_max, NULL) &&
    cl_heap_pop(values, count, sizeof(values[0]), compare_int_max, NULL)) {
    --count;
    /* values[count] is the former highest-priority element. */
}
```

## Comparator Contract

The comparator receives two element pointers and the user pointer passed to the
heap operation. It must return a positive value when the left element has higher
priority than the right element, a negative value when it has lower priority,
and zero when the elements are equivalent for heap ordering.

Reverse the comparator to build a min-heap.

## Operations

- `cl_heap_make` transforms an existing array into a heap in O(n).
- `cl_heap_push` assumes `base[0..count - 2]` is already a heap and sifts the
  appended last element into place.
- `cl_heap_pop` swaps the root with the last active element, restores the heap
  over `count - 1` elements, and leaves the removed root in the final slot.
- `cl_heap_sort` heapifies and sorts in place. With a max comparator the result
  is ordered from lowest to highest priority.
- `cl_heap_is_valid` checks the heap invariant without mutating the array.

## Contracts

- `base` must point to at least `count * element_size` bytes whenever `count` is
  non-zero.
- `element_size` must match the objects stored in the array.
- `cl_heap_push` requires the caller to append the new element and pass the new
  count.
- `cl_heap_pop` requires the caller to decrement its active count after a
  successful pop if it wants to remove the final slot from the heap.
- The comparator must impose a consistent ordering while an operation is
  running.

## Safety Properties

- The implementation performs no allocation and cannot fail due to allocator
  state.
- Public operations reject null non-empty arrays, zero-sized elements, null
  comparators, and `count * element_size` values that overflow `size_t`.
- Child index arithmetic is guarded so it does not overflow while walking very
  large heap counts.
- Swaps are byte-wise, so the algorithms work with ordinary C object
  representations stored in arrays.

## Portability

The implementation targets POSIX.1-2008 compatible C99 and uses only standard C
pointer arithmetic within caller-provided arrays.
