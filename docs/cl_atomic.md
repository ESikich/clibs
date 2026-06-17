<!--
docs/cl_atomic.md
Purpose: Atomic wrapper library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_atomic

`cl_atomic` provides a small C99-friendly wrapper around GCC/Clang `__atomic`
builtins for shared integer and pointer state.

## Interface

```c
#define CL_ATOMIC_HAS_BUILTINS 1

typedef enum cl_memory_order {
    CL_MEMORY_ORDER_RELAXED,
    CL_MEMORY_ORDER_CONSUME,
    CL_MEMORY_ORDER_ACQUIRE,
    CL_MEMORY_ORDER_RELEASE,
    CL_MEMORY_ORDER_ACQ_REL,
    CL_MEMORY_ORDER_SEQ_CST
} cl_memory_order;

typedef struct cl_atomic_u32 cl_atomic_u32;
typedef struct cl_atomic_u64 cl_atomic_u64;
typedef struct cl_atomic_size cl_atomic_size;
typedef struct cl_atomic_ptr cl_atomic_ptr;
```

Each unsigned integer type has the same operation family:

```c
void cl_atomic_u32_init(cl_atomic_u32 *atomic, uint32_t value);
uint32_t cl_atomic_u32_load(const cl_atomic_u32 *atomic, cl_memory_order order);
void cl_atomic_u32_store(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_exchange(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
bool cl_atomic_u32_compare_exchange(
    cl_atomic_u32 *atomic,
    uint32_t *expected,
    uint32_t desired,
    cl_memory_order success_order,
    cl_memory_order failure_order);
uint32_t cl_atomic_u32_fetch_add(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_sub(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_or(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_and(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
uint32_t cl_atomic_u32_fetch_xor(cl_atomic_u32 *atomic, uint32_t value, cl_memory_order order);
```

`cl_atomic_u64` and `cl_atomic_size` expose matching signatures with their own
value types. `cl_atomic_ptr` supports init, load, store, exchange, and
compare-exchange for `void *` values.

```c
void cl_atomic_thread_fence(cl_memory_order order);
void cl_atomic_signal_fence(cl_memory_order order);
```

## Contracts

Callers must pass non-null atomic objects to every operation. Compare-exchange
also requires a non-null `expected` pointer. On compare-exchange failure, the
current value is written back through `expected`, matching the compiler builtin
contract.

The `init` helpers perform plain initialization and are intended for objects
that are not concurrently visible yet. Once an object is shared, use `store` or
`exchange` to publish new values.

Fetch operations return the previous value. Arithmetic on unsigned atomic
wrappers follows the underlying unsigned type's wraparound rules.

Memory orders map directly to the compiler's `__ATOMIC_*` constants. Use
`CL_MEMORY_ORDER_SEQ_CST` unless a weaker ordering is part of a documented
concurrency design. Failure orders for compare-exchange should not be release or
acq-rel orders.

## Portability

C99 and POSIX.1-2008 do not provide a standard atomic API. This module requires
GCC- or Clang-compatible `__atomic` builtins and fails at compile time otherwise.
`CL_ATOMIC_HAS_BUILTINS` is exposed so callers can make the same decision in
their own conditional code.

The wrappers do not promise that operations are lock-free on every target. They
preserve the compiler builtin semantics for the selected object size and target
ABI.
