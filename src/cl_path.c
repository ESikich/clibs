/*
 * cl_path.c
 * Purpose: Implement lexical POSIX path helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_path.h"

#include <stdint.h>

static size_t cl_path_strlen(const char *s)
{
    size_t size = 0u;

    if (!s) {
        return 0u;
    }

    while (s[size] != '\0') {
        ++size;
    }
    return size;
}

static bool cl_path_is_dot(const char *data, size_t size)
{
    return size == 1u && data[0] == '.';
}

static bool cl_path_is_dotdot(const char *data, size_t size)
{
    return size == 2u && data[0] == '.' && data[1] == '.';
}

static bool cl_path_append_char(char *out, size_t out_size, size_t *size, char c)
{
    if (!out || !size || *size + 1u >= out_size) {
        return false;
    }

    out[*size] = c;
    ++*size;
    return true;
}

static bool cl_path_append_component(
    char *out,
    size_t out_size,
    size_t *size,
    bool absolute,
    const char *component,
    size_t component_size)
{
    size_t i;

    if (!out || !size || (!component && component_size != 0u)) {
        return false;
    }

    if (*size != 0u && !(absolute && *size == 1u)) {
        if (!cl_path_append_char(out, out_size, size, '/')) {
            return false;
        }
    }

    for (i = 0u; i < component_size; ++i) {
        if (!cl_path_append_char(out, out_size, size, component[i])) {
            return false;
        }
    }

    return true;
}

static size_t cl_path_last_component_start(const char *out, size_t size)
{
    size_t start = size;

    while (start > 0u && out[start - 1u] != '/') {
        --start;
    }
    return start;
}

static bool cl_path_output_ends_with_parent_ref(const char *out, size_t size)
{
    size_t start;

    if (size == 0u) {
        return false;
    }

    start = cl_path_last_component_start(out, size);
    return cl_path_is_dotdot(out + start, size - start);
}

static void cl_path_pop_last_component(char *out, size_t *size, bool absolute)
{
    size_t start;

    if (!out || !size || *size == 0u || (absolute && *size == 1u)) {
        return;
    }

    start = cl_path_last_component_start(out, *size);
    if (start > 0u && out[start - 1u] == '/') {
        --start;
    }
    *size = start;
    if (absolute && *size < 1u) {
        *size = 1u;
    }
}

static bool cl_path_normalize_range(
    const char *path,
    size_t path_size,
    char *out,
    size_t out_size,
    size_t *written)
{
    size_t out_len = 0u;
    size_t i = 0u;
    bool absolute;

    if (!path || !out || out_size == 0u) {
        return false;
    }

    absolute = path_size != 0u && path[0] == '/';
    if (absolute) {
        if (!cl_path_append_char(out, out_size, &out_len, '/')) {
            return false;
        }
    }

    while (i < path_size) {
        size_t start;
        size_t component_size;

        while (i < path_size && path[i] == '/') {
            ++i;
        }
        start = i;
        while (i < path_size && path[i] != '/') {
            ++i;
        }
        component_size = i - start;
        if (component_size == 0u || cl_path_is_dot(path + start, component_size)) {
            continue;
        }

        if (cl_path_is_dotdot(path + start, component_size)) {
            if (absolute) {
                cl_path_pop_last_component(out, &out_len, absolute);
                continue;
            }
            if (out_len != 0u && !cl_path_output_ends_with_parent_ref(out, out_len)) {
                cl_path_pop_last_component(out, &out_len, absolute);
                continue;
            }
            if (!cl_path_append_component(
                    out,
                    out_size,
                    &out_len,
                    absolute,
                    "..",
                    2u)) {
                return false;
            }
            continue;
        }

        if (!cl_path_append_component(
                out,
                out_size,
                &out_len,
                absolute,
                path + start,
                component_size)) {
            return false;
        }
    }

    if (out_len == 0u) {
        if (!cl_path_append_char(out, out_size, &out_len, '.')) {
            return false;
        }
    }

    out[out_len] = '\0';
    if (written) {
        *written = out_len;
    }
    return true;
}

static cl_path_view cl_path_static_view(const char *s)
{
    cl_path_view view;

    view.data = s;
    view.size = cl_path_strlen(s);
    return view;
}

bool cl_path_normalize(const char *path, char *out, size_t out_size, size_t *written)
{
    if (!path) {
        return false;
    }

    return cl_path_normalize_range(path, cl_path_strlen(path), out, out_size, written);
}

bool cl_path_join(
    const char *base,
    const char *child,
    char *out,
    size_t out_size,
    size_t *written)
{
    size_t base_size;
    size_t child_size;

    if (!base || !child || !out) {
        return false;
    }

    child_size = cl_path_strlen(child);
    if (child_size != 0u && child[0] == '/') {
        return cl_path_normalize(child, out, out_size, written);
    }

    base_size = cl_path_strlen(base);
    if (base_size > SIZE_MAX - child_size - 2u) {
        return false;
    }

    {
        char joined[base_size + child_size + 2u];
        size_t joined_size = 0u;
        size_t i;

        for (i = 0u; i < base_size; ++i) {
            joined[joined_size++] = base[i];
        }
        if (base_size != 0u && child_size != 0u &&
            joined[joined_size - 1u] != '/') {
            joined[joined_size++] = '/';
        }
        for (i = 0u; i < child_size; ++i) {
            joined[joined_size++] = child[i];
        }
        joined[joined_size] = '\0';

        return cl_path_normalize_range(joined, joined_size, out, out_size, written);
    }
}

cl_path_view cl_path_basename(const char *path)
{
    cl_path_view view;
    size_t end;
    size_t start;

    if (!path || path[0] == '\0') {
        return cl_path_static_view(".");
    }

    end = cl_path_strlen(path);
    while (end > 1u && path[end - 1u] == '/') {
        --end;
    }
    if (end == 1u && path[0] == '/') {
        return cl_path_static_view("/");
    }

    start = end;
    while (start > 0u && path[start - 1u] != '/') {
        --start;
    }

    view.data = path + start;
    view.size = end - start;
    return view;
}

cl_path_view cl_path_dirname(const char *path)
{
    cl_path_view view;
    size_t end;

    if (!path || path[0] == '\0') {
        return cl_path_static_view(".");
    }

    end = cl_path_strlen(path);
    while (end > 1u && path[end - 1u] == '/') {
        --end;
    }
    while (end > 0u && path[end - 1u] != '/') {
        --end;
    }
    while (end > 1u && path[end - 1u] == '/') {
        --end;
    }

    if (end == 0u) {
        return cl_path_static_view(".");
    }
    if (end == 1u && path[0] == '/') {
        return cl_path_static_view("/");
    }

    view.data = path;
    view.size = end;
    return view;
}
