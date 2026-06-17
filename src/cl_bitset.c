/*
 * cl_bitset.c
 * Purpose: Implement fixed-storage and allocator-backed bitset helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_bitset.h"

#include <stdint.h>
#include <string.h>

#define CL_BITSET_WORD_BITS (sizeof(size_t) * 8u)

static size_t cl_bitset_word_count(size_t bit_count)
{
    return bit_count == 0u
        ? 0u
        : ((bit_count - 1u) / CL_BITSET_WORD_BITS) + 1u;
}

static size_t cl_bitset_word_mask(size_t index)
{
    return (size_t)1u << (index % CL_BITSET_WORD_BITS);
}

static size_t cl_bitset_last_word_mask(size_t bit_count)
{
    size_t used_bits;

    if (bit_count == 0u) {
        return 0u;
    }

    used_bits = bit_count % CL_BITSET_WORD_BITS;
    if (used_bits == 0u) {
        return ~(size_t)0u;
    }

    return (((size_t)1u << used_bits) - 1u);
}

static void cl_bitset_mask_unused_bits(cl_bitset *set)
{
    if (!set || set->word_count == 0u) {
        return;
    }

    set->words[set->word_count - 1u] &= cl_bitset_last_word_mask(set->bit_count);
}

static size_t cl_bitset_popcount_word(size_t word)
{
    size_t count = 0u;

    while (word != 0u) {
        word &= word - 1u;
        ++count;
    }

    return count;
}

static bool cl_bitset_find_in_word(
    size_t word,
    size_t base,
    size_t limit,
    size_t *out_index)
{
    size_t bit;

    if (word == 0u || !out_index) {
        return false;
    }

    for (bit = 0u; bit < CL_BITSET_WORD_BITS && base + bit < limit; ++bit) {
        if ((word & ((size_t)1u << bit)) != 0u) {
            *out_index = base + bit;
            return true;
        }
    }

    return false;
}

static bool cl_bitset_compatible(const cl_bitset *a, const cl_bitset *b)
{
    return a && b && a->bit_count == b->bit_count &&
           a->word_count == b->word_count &&
           (a->word_count == 0u || (a->words && b->words));
}

static bool cl_bitset_storage_is_aligned(const void *storage)
{
    return ((uintptr_t)storage % sizeof(size_t)) == 0u;
}

bool cl_bitset_required_bytes(size_t bit_count, size_t *out_bytes)
{
    size_t words;

    if (!out_bytes) {
        return false;
    }

    words = cl_bitset_word_count(bit_count);
    if (words > SIZE_MAX / sizeof(size_t)) {
        return false;
    }

    *out_bytes = words * sizeof(size_t);
    return true;
}

bool cl_bitset_init(cl_bitset *set, cl_allocator *allocator, size_t bit_count)
{
    size_t bytes;

    if (!set || !cl_bitset_required_bytes(bit_count, &bytes)) {
        return false;
    }

    set->words = NULL;
    set->bit_count = bit_count;
    set->word_count = cl_bitset_word_count(bit_count);
    set->allocator = allocator;

    if (bytes == 0u) {
        return true;
    }

    set->words = cl_alloc(allocator, bytes, sizeof(size_t));
    if (!set->words) {
        set->bit_count = 0u;
        set->word_count = 0u;
        set->allocator = NULL;
        return false;
    }

    (void)memset(set->words, 0, bytes);
    return true;
}

bool cl_bitset_init_with_storage(
    cl_bitset *set,
    void *storage,
    size_t storage_size,
    size_t bit_count)
{
    size_t bytes;

    if (!set || !cl_bitset_required_bytes(bit_count, &bytes)) {
        return false;
    }
    if (bytes != 0u &&
        (!storage || storage_size < bytes ||
         !cl_bitset_storage_is_aligned(storage))) {
        return false;
    }

    set->words = (size_t *)storage;
    set->bit_count = bit_count;
    set->word_count = cl_bitset_word_count(bit_count);
    set->allocator = NULL;

    if (bytes != 0u) {
        (void)memset(set->words, 0, bytes);
    }
    return true;
}

void cl_bitset_free(cl_bitset *set)
{
    size_t bytes;

    if (!set) {
        return;
    }

    if (set->allocator && cl_bitset_required_bytes(set->bit_count, &bytes)) {
        cl_free(set->allocator, set->words, bytes, sizeof(size_t));
    }

    set->words = NULL;
    set->bit_count = 0u;
    set->word_count = 0u;
    set->allocator = NULL;
}

size_t cl_bitset_size(const cl_bitset *set)
{
    return set ? set->bit_count : 0u;
}

bool cl_bitset_get(const cl_bitset *set, size_t index, bool *out)
{
    if (!set || !out || index >= set->bit_count || !set->words) {
        return false;
    }

    *out = (set->words[index / CL_BITSET_WORD_BITS] &
            cl_bitset_word_mask(index)) != 0u;
    return true;
}

bool cl_bitset_set(cl_bitset *set, size_t index)
{
    if (!set || index >= set->bit_count || !set->words) {
        return false;
    }

    set->words[index / CL_BITSET_WORD_BITS] |= cl_bitset_word_mask(index);
    return true;
}

bool cl_bitset_clear(cl_bitset *set, size_t index)
{
    if (!set || index >= set->bit_count || !set->words) {
        return false;
    }

    set->words[index / CL_BITSET_WORD_BITS] &= ~cl_bitset_word_mask(index);
    return true;
}

bool cl_bitset_toggle(cl_bitset *set, size_t index)
{
    if (!set || index >= set->bit_count || !set->words) {
        return false;
    }

    set->words[index / CL_BITSET_WORD_BITS] ^= cl_bitset_word_mask(index);
    return true;
}

void cl_bitset_clear_all(cl_bitset *set)
{
    if (!set || !set->words) {
        return;
    }

    (void)memset(set->words, 0, set->word_count * sizeof(size_t));
}

void cl_bitset_fill_all(cl_bitset *set)
{
    if (!set || !set->words) {
        return;
    }

    (void)memset(set->words, 0xff, set->word_count * sizeof(size_t));
    cl_bitset_mask_unused_bits(set);
}

size_t cl_bitset_count(const cl_bitset *set)
{
    size_t count = 0u;
    size_t i;

    if (!set || !set->words) {
        return 0u;
    }

    for (i = 0u; i < set->word_count; ++i) {
        count += cl_bitset_popcount_word(set->words[i]);
    }
    return count;
}

bool cl_bitset_find_first_set(
    const cl_bitset *set,
    size_t start,
    size_t *out_index)
{
    size_t word_index;
    size_t word;
    size_t base;

    if (!set || !set->words || !out_index || start >= set->bit_count) {
        return false;
    }

    word_index = start / CL_BITSET_WORD_BITS;
    word = set->words[word_index];
    word &= ~(cl_bitset_word_mask(start) - 1u);

    for (;;) {
        base = word_index * CL_BITSET_WORD_BITS;
        if (cl_bitset_find_in_word(word, base, set->bit_count, out_index)) {
            return true;
        }

        ++word_index;
        if (word_index >= set->word_count) {
            return false;
        }
        word = set->words[word_index];
    }
}

bool cl_bitset_find_first_clear(
    const cl_bitset *set,
    size_t start,
    size_t *out_index)
{
    size_t word_index;
    size_t word;
    size_t base;

    if (!set || !set->words || !out_index || start >= set->bit_count) {
        return false;
    }

    word_index = start / CL_BITSET_WORD_BITS;
    word = ~set->words[word_index];
    word &= ~(cl_bitset_word_mask(start) - 1u);

    for (;;) {
        base = word_index * CL_BITSET_WORD_BITS;
        if (word_index + 1u == set->word_count) {
            word &= cl_bitset_last_word_mask(set->bit_count);
        }
        if (cl_bitset_find_in_word(word, base, set->bit_count, out_index)) {
            return true;
        }

        ++word_index;
        if (word_index >= set->word_count) {
            return false;
        }
        word = ~set->words[word_index];
    }
}

bool cl_bitset_and(cl_bitset *dst, const cl_bitset *rhs)
{
    size_t i;

    if (!cl_bitset_compatible(dst, rhs)) {
        return false;
    }

    for (i = 0u; i < dst->word_count; ++i) {
        dst->words[i] &= rhs->words[i];
    }
    cl_bitset_mask_unused_bits(dst);
    return true;
}

bool cl_bitset_or(cl_bitset *dst, const cl_bitset *rhs)
{
    size_t i;

    if (!cl_bitset_compatible(dst, rhs)) {
        return false;
    }

    for (i = 0u; i < dst->word_count; ++i) {
        dst->words[i] |= rhs->words[i];
    }
    cl_bitset_mask_unused_bits(dst);
    return true;
}

bool cl_bitset_xor(cl_bitset *dst, const cl_bitset *rhs)
{
    size_t i;

    if (!cl_bitset_compatible(dst, rhs)) {
        return false;
    }

    for (i = 0u; i < dst->word_count; ++i) {
        dst->words[i] ^= rhs->words[i];
    }
    cl_bitset_mask_unused_bits(dst);
    return true;
}

void cl_bitset_not(cl_bitset *set)
{
    size_t i;

    if (!set || !set->words) {
        return;
    }

    for (i = 0u; i < set->word_count; ++i) {
        set->words[i] = ~set->words[i];
    }
    cl_bitset_mask_unused_bits(set);
}
