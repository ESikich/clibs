<!--
docs/cl_bitset.md
Purpose: Bitset library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# cl_bitset

`cl_bitset` provides fixed-storage and allocator-backed bitsets over `size_t`
words. It supports range-checked bit access, whole-set boolean operations,
population counts, and first-set/first-clear scans.

## Interface

```c
typedef struct cl_bitset {
    size_t *words;
    size_t bit_count;
    size_t word_count;
    cl_allocator *allocator;
} cl_bitset;

bool cl_bitset_required_bytes(size_t bit_count, size_t *out_bytes);
bool cl_bitset_init(cl_bitset *set, cl_allocator *allocator, size_t bit_count);
bool cl_bitset_init_with_storage(
    cl_bitset *set,
    void *storage,
    size_t storage_size,
    size_t bit_count);
void cl_bitset_free(cl_bitset *set);

size_t cl_bitset_size(const cl_bitset *set);
bool cl_bitset_get(const cl_bitset *set, size_t index, bool *out);
bool cl_bitset_set(cl_bitset *set, size_t index);
bool cl_bitset_clear(cl_bitset *set, size_t index);
bool cl_bitset_toggle(cl_bitset *set, size_t index);
void cl_bitset_clear_all(cl_bitset *set);
void cl_bitset_fill_all(cl_bitset *set);

size_t cl_bitset_count(const cl_bitset *set);
bool cl_bitset_find_first_set(
    const cl_bitset *set,
    size_t start,
    size_t *out_index);
bool cl_bitset_find_first_clear(
    const cl_bitset *set,
    size_t start,
    size_t *out_index);

bool cl_bitset_and(cl_bitset *dst, const cl_bitset *rhs);
bool cl_bitset_or(cl_bitset *dst, const cl_bitset *rhs);
bool cl_bitset_xor(cl_bitset *dst, const cl_bitset *rhs);
void cl_bitset_not(cl_bitset *set);
```

## Contracts

`cl_bitset_required_bytes` reports the caller-owned storage required for a bit
count. `cl_bitset_init_with_storage` uses that caller-owned storage and never
frees it; non-empty storage must be aligned for `size_t`, so a `size_t[]` buffer
is the simplest fixed-storage backing. `cl_bitset_init` allocates the storage
through the supplied `cl_allocator`; `cl_bitset_free` releases only
allocator-backed storage and resets the struct to empty.

All indexed operations validate the bit index. `cl_bitset_get` writes through
`out` only on success. Mutation helpers return `false` for null sets, empty
sets, and out-of-range indexes.

`cl_bitset_fill_all`, `cl_bitset_not`, and boolean operations mask unused bits in
the final word so scans and counts only observe bits below `bit_count`.

Boolean operations are in-place on `dst` and require both bitsets to have the
same `bit_count`. They return `false` without mutating when sizes do not match.

## Portability

The implementation uses C99 and POSIX-compatible library calls only. It does not
depend on compiler-specific popcount or bit-scan intrinsics, which keeps the
same behavior across POSIX.1-2008 targets.
