/*
 * cl_file.c
 * Purpose: Whole-file read/write helper implementation.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "cl_file.h"

#include "cl_buffer.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static bool cl_file_add_terminator_capacity(size_t size, size_t *out)
{
    if (!out || size == SIZE_MAX) {
        return false;
    }

    *out = size + 1u;
    return true;
}

static bool cl_file_size_from_stat(const struct stat *st, size_t *out)
{
    if (!st || !out || st->st_size < 0) {
        return false;
    }

    if ((uintmax_t)st->st_size > (uintmax_t)SIZE_MAX) {
        return false;
    }

    *out = (size_t)st->st_size;
    return true;
}

static bool cl_file_close_success(int fd, bool ok)
{
    int saved_errno = errno;

    if (close(fd) != 0) {
        ok = false;
    }

    errno = saved_errno;
    return ok;
}

static bool cl_file_read_exact(int fd, unsigned char *data, size_t size)
{
    size_t offset = 0u;

    while (offset < size) {
        size_t remaining = size - offset;
        ssize_t nread;

        if (remaining > (size_t)SSIZE_MAX) {
            remaining = (size_t)SSIZE_MAX;
        }

        nread = read(fd, data + offset, remaining);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        if (nread == 0) {
            return false;
        }
        offset += (size_t)nread;
    }

    return true;
}

static bool cl_file_read_stream(int fd, cl_allocator *allocator, cl_file_data *out)
{
    unsigned char chunk[4096];
    cl_buffer buffer;

    cl_buffer_init(&buffer, allocator);

    for (;;) {
        ssize_t nread = read(fd, chunk, sizeof(chunk));

        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            }
            cl_buffer_free(&buffer);
            return false;
        }
        if (nread == 0) {
            break;
        }
        if (!cl_buffer_append(&buffer, chunk, (size_t)nread)) {
            cl_buffer_free(&buffer);
            return false;
        }
    }

    if (!cl_buffer_append_byte(&buffer, 0u)) {
        cl_buffer_free(&buffer);
        return false;
    }

    out->data = buffer.data;
    out->size = buffer.size - 1u;
    out->capacity = buffer.capacity;
    out->allocator = allocator;
    return true;
}

void cl_file_data_init(cl_file_data *file, cl_allocator *allocator)
{
    if (!file) {
        return;
    }

    file->data = NULL;
    file->size = 0u;
    file->capacity = 0u;
    file->allocator = allocator;
}

void cl_file_data_free(cl_file_data *file)
{
    if (!file) {
        return;
    }

    cl_free(file->allocator, file->data, file->capacity, 1u);
    file->data = NULL;
    file->size = 0u;
    file->capacity = 0u;
}

bool cl_file_read_all(const char *path, cl_allocator *allocator, cl_file_data *out)
{
    struct stat st;
    bool ok = false;
    int fd;

    if (!path || !allocator || !out) {
        return false;
    }

    cl_file_data_init(out, allocator);

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    if (fstat(fd, &st) != 0) {
        return cl_file_close_success(fd, false);
    }

    if (S_ISREG(st.st_mode)) {
        size_t size;
        size_t capacity;

        if (!cl_file_size_from_stat(&st, &size) ||
            !cl_file_add_terminator_capacity(size, &capacity)) {
            return cl_file_close_success(fd, false);
        }

        out->data = cl_alloc(allocator, capacity, 1u);
        if (!out->data) {
            return cl_file_close_success(fd, false);
        }
        out->capacity = capacity;
        out->allocator = allocator;

        ok = cl_file_read_exact(fd, out->data, size);
        if (ok) {
            out->data[size] = 0u;
            out->size = size;
        }
    } else {
        ok = cl_file_read_stream(fd, allocator, out);
    }

    ok = cl_file_close_success(fd, ok);
    if (!ok) {
        cl_file_data_free(out);
    }
    return ok;
}

bool cl_file_write_all(const char *path, const void *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t offset = 0u;
    int fd;
    bool ok = true;

    if (!path || (size != 0u && !data)) {
        return false;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        return false;
    }

    while (offset < size) {
        size_t remaining = size - offset;
        ssize_t nwritten;

        if (remaining > (size_t)SSIZE_MAX) {
            remaining = (size_t)SSIZE_MAX;
        }

        nwritten = write(fd, bytes + offset, remaining);
        if (nwritten < 0) {
            if (errno == EINTR) {
                continue;
            }
            ok = false;
            break;
        }
        if (nwritten == 0) {
            ok = false;
            break;
        }
        offset += (size_t)nwritten;
    }

    return cl_file_close_success(fd, ok);
}
