/*
 * cl_path.h
 * Purpose: Lexical POSIX path helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef CL_PATH_H
#define CL_PATH_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_path_view {
    const char *data;
    size_t size;
} cl_path_view;

bool cl_path_normalize(const char *path, char *out, size_t out_size, size_t *written);
bool cl_path_join(
    const char *base,
    const char *child,
    char *out,
    size_t out_size,
    size_t *written);
cl_path_view cl_path_basename(const char *path);
cl_path_view cl_path_dirname(const char *path);

#ifdef __cplusplus
}
#endif

#endif
