/*
 * cl_file.h
 * Purpose: Whole-file read/write helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_FILE_H
#define CL_FILE_H

#include "cl_alloc.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_file_data {
    unsigned char *data;
    size_t size;
    size_t capacity;
    cl_allocator *allocator;
} cl_file_data;

void cl_file_data_init(cl_file_data *file, cl_allocator *allocator);
void cl_file_data_free(cl_file_data *file);
bool cl_file_read_all(const char *path, cl_allocator *allocator, cl_file_data *out);
bool cl_file_write_all(const char *path, const void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
