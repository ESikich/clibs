<!--
docs/cl_array.md
Purpose: Dynamic array library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_array

`cl_array` provides macro-generated typed dynamic arrays backed by `cl_allocator`.
The generated array owns its storage, exposes `data`, `size`, and `capacity`, and
keeps growth arithmetic checked before allocating.

## Defining an Array

Define one array type per element type:

```c
#include "cl_array.h"

CL_ARRAY_DEFINE(int_array, int)
```

This creates `int_array` plus these inline operations:

- `int_array_init`
- `int_array_clear`
- `int_array_free`
- `int_array_reserve`
- `int_array_resize`
- `int_array_push`
- `int_array_pop`

## Example

```c
cl_allocator allocator = cl_system_allocator();
int_array values;

int_array_init(&values, &allocator);

if (int_array_push(&values, 42)) {
    values.data[0] += 1;
}

int_array_free(&values);
```

## Contracts

- The allocator passed to `init` must outlive the array.
- `free` releases the backing allocation and resets `data`, `size`, and
  `capacity`.
- `clear` drops all elements without releasing capacity.
- `reserve` grows capacity to at least the requested value.
- `resize` grows as needed and zero-initializes newly exposed elements.
- `push` appends one value.
- `pop` removes the last value and optionally copies it to `out`.

## Safety Properties

- Capacity growth uses checked multiplication before each allocator operation.
- Failed growth leaves the original allocation, size, and capacity unchanged.
- New elements created by `resize` are zero-initialized for predictable contents.
- Allocation and free calls pass the element type's size and alignment through
  the explicit allocator interface.

## Portability

The implementation targets POSIX.1-2008 compatible C99. Array operation
generation uses C99 `static inline` functions and `offsetof` to compute element
alignment without compiler-specific extensions.
