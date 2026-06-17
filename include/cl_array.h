/*
 * cl_array.h
 * Purpose: Macro-generated typed dynamic arrays for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_ARRAY_H
#define CL_ARRAY_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CL_ALIGNOF_TYPE(type) offsetof(struct { char c; type member; }, member)

bool cl_array_allocation_size(size_t capacity, size_t elem_size, size_t *out);
bool cl_array_next_capacity(
    size_t current_capacity,
    size_t min_capacity,
    size_t *out_capacity);
bool cl_array_reallocate(
    cl_allocator *allocator,
    void **data,
    size_t old_capacity,
    size_t new_capacity,
    size_t elem_size,
    size_t elem_align);

/*
 * Define a typed array and its operations. The array owns its data through the
 * allocator passed to init; callers may read/write data[0..size) directly.
 */
#define CL_ARRAY_DEFINE(name, type)                                             \
    typedef struct name {                                                       \
        type *data;                                                             \
        size_t size;                                                            \
        size_t capacity;                                                        \
        cl_allocator *allocator;                                                \
    } name;                                                                     \
                                                                                 \
    static inline void name##_init(name *array, cl_allocator *allocator)        \
    {                                                                           \
        if (!array) {                                                           \
            return;                                                             \
        }                                                                       \
        array->data = NULL;                                                     \
        array->size = 0u;                                                       \
        array->capacity = 0u;                                                   \
        array->allocator = allocator;                                           \
    }                                                                           \
                                                                                 \
    static inline void name##_clear(name *array)                                \
    {                                                                           \
        if (!array) {                                                           \
            return;                                                             \
        }                                                                       \
        array->size = 0u;                                                       \
    }                                                                           \
                                                                                 \
    static inline void name##_free(name *array)                                 \
    {                                                                           \
        void *data;                                                             \
        if (!array) {                                                           \
            return;                                                             \
        }                                                                       \
        data = array->data;                                                     \
        (void)cl_array_reallocate(                                              \
            array->allocator,                                                   \
            &data,                                                              \
            array->capacity,                                                    \
            0u,                                                                 \
            sizeof(type),                                                       \
            CL_ALIGNOF_TYPE(type));                                             \
        array->data = NULL;                                                     \
        array->size = 0u;                                                       \
        array->capacity = 0u;                                                   \
    }                                                                           \
                                                                                 \
    static inline bool name##_reserve(name *array, size_t min_capacity)         \
    {                                                                           \
        void *data;                                                             \
        size_t next_capacity;                                                   \
        if (!array) {                                                           \
            return false;                                                       \
        }                                                                       \
        if (min_capacity <= array->capacity) {                                  \
            return true;                                                        \
        }                                                                       \
        if (!cl_array_next_capacity(                                            \
                array->capacity, min_capacity, &next_capacity)) {               \
            return false;                                                       \
        }                                                                       \
        data = array->data;                                                     \
        if (!cl_array_reallocate(                                               \
                array->allocator,                                               \
                &data,                                                          \
                array->capacity,                                                \
                next_capacity,                                                  \
                sizeof(type),                                                   \
                CL_ALIGNOF_TYPE(type))) {                                       \
            return false;                                                       \
        }                                                                       \
        array->data = (type *)data;                                             \
        array->capacity = next_capacity;                                        \
        return true;                                                            \
    }                                                                           \
                                                                                 \
    static inline bool name##_resize(name *array, size_t new_size)              \
    {                                                                           \
        if (!array) {                                                           \
            return false;                                                       \
        }                                                                       \
        if (new_size > array->capacity && !name##_reserve(array, new_size)) {   \
            return false;                                                       \
        }                                                                       \
        if (new_size > array->size) {                                           \
            memset(                                                             \
                array->data + array->size,                                      \
                0,                                                              \
                (new_size - array->size) * sizeof(type));                       \
        }                                                                       \
        array->size = new_size;                                                 \
        return true;                                                            \
    }                                                                           \
                                                                                 \
    static inline bool name##_push(name *array, type value)                     \
    {                                                                           \
        if (!array) {                                                           \
            return false;                                                       \
        }                                                                       \
        if (array->size == (size_t)-1) {                                        \
            return false;                                                       \
        }                                                                       \
        if (array->size == array->capacity &&                                  \
            !name##_reserve(array, array->size + 1u)) {                         \
            return false;                                                       \
        }                                                                       \
        array->data[array->size] = value;                                       \
        ++array->size;                                                          \
        return true;                                                            \
    }                                                                           \
                                                                                 \
    static inline bool name##_pop(name *array, type *out)                       \
    {                                                                           \
        if (!array || array->size == 0u) {                                      \
            return false;                                                       \
        }                                                                       \
        --array->size;                                                          \
        if (out) {                                                              \
            *out = array->data[array->size];                                    \
        }                                                                       \
        return true;                                                            \
    }

#ifdef __cplusplus
}
#endif

#endif
