/*
 * overview.c
 * Purpose: End-to-end example using the core clibs libraries together.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-17.
 */

#include "cl_alloc.h"
#include "cl_array.h"
#include "cl_buffer.h"
#include "cl_file.h"
#include "cl_hash.h"
#include "cl_libc.h"
#include "cl_path.h"
#include "cl_sv.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef struct item {
    char name[16];
    uint64_t count;
} item;

typedef struct event {
    const item *source;
    uint64_t doubled;
} event;

CL_ARRAY_DEFINE(item_array, item)

static bool copy_sv_to_cstr(cl_sv src, char *dst, size_t dst_size)
{
    if (!dst || dst_size == 0u || src.size >= dst_size) {
        return false;
    }
    if (src.size != 0u && !src.data) {
        return false;
    }

    (void)cl_memset(dst, 0, dst_size);
    (void)cl_memcpy(dst, src.data, src.size);
    return true;
}

static bool parse_item(cl_sv field, item *out)
{
    cl_sv name;
    cl_sv count_text;
    uint64_t count = 0u;
    item parsed;

    if (!out || !cl_sv_split_once(field, ':', &name, &count_text)) {
        return false;
    }

    name = cl_sv_trim(name);
    count_text = cl_sv_trim(count_text);
    if (name.size == 0u ||
        cl_sv_parse_u64(count_text, &count) != CL_SV_PARSE_OK) {
        return false;
    }

    if (!copy_sv_to_cstr(name, parsed.name, sizeof(parsed.name))) {
        return false;
    }
    parsed.count = count;
    *out = parsed;
    return true;
}

static bool parse_items(cl_sv text, item_array *items)
{
    cl_sv rest = text;
    cl_sv field;

    if (!items) {
        return false;
    }

    while (cl_sv_next_split(&rest, ',', &field)) {
        item parsed;

        field = cl_sv_trim(field);
        if (field.size == 0u) {
            continue;
        }
        if (!parse_item(field, &parsed) || !item_array_push(items, parsed)) {
            return false;
        }
    }

    return true;
}

static void compact_items(item_array *items)
{
    size_t read_index;
    size_t write_index = 0u;

    if (!items) {
        return;
    }

    for (read_index = 0u; read_index < items->size; ++read_index) {
        if (items->data[read_index].count == 0u) {
            continue;
        }
        if (write_index != read_index) {
            (void)cl_memmove(
                &items->data[write_index],
                &items->data[read_index],
                sizeof(items->data[write_index]));
        }
        ++write_index;
    }

    items->size = write_index;
}

static uint64_t total_count(const item_array *items)
{
    uint64_t total = 0u;
    size_t i;

    if (!items) {
        return 0u;
    }

    for (i = 0u; i < items->size; ++i) {
        total += items->data[i].count;
    }
    return total;
}

static bool index_items(const item_array *items, cl_hash_table *index)
{
    size_t i;

    if (!items || !index) {
        return false;
    }

    for (i = 0u; i < items->size; ++i) {
        if (!cl_hash_table_put_cstr(
                index,
                items->data[i].name,
                (void *)&items->data[i])) {
            return false;
        }
    }

    return true;
}

static void make_example_path(char *path, size_t path_size)
{
    if (!path || path_size == 0u) {
        return;
    }

    (void)snprintf(path, path_size, "/tmp/clibs_overview_%ld.txt", (long)getpid());
}

static void print_report(const item_array *items, cl_arena *scratch)
{
    char *line;
    cl_allocator allocator;
    size_t i;

    if (!items || !scratch) {
        return;
    }

    allocator = cl_arena_allocator(scratch);
    line = cl_alloc(&allocator, 80u, 16u);
    if (!line) {
        puts("scratch allocation failed");
        return;
    }

    line = cl_resize(&allocator, line, 80u, 128u, 16u);
    if (!line) {
        puts("scratch resize failed");
        return;
    }

    for (i = 0u; i < items->size; ++i) {
        (void)snprintf(
            line,
            128u,
            "%-10s %" PRIu64,
            items->data[i].name,
            items->data[i].count);
        puts(line);
    }
}

static bool queue_event(cl_allocator *allocator, const item *source)
{
    event *queued;

    if (!allocator || !source) {
        return false;
    }

    queued = cl_alloc(allocator, sizeof(*queued), CL_ALIGNOF_TYPE(event));
    if (!queued) {
        return false;
    }

    queued->source = source;
    queued->doubled = source->count * 2u;
    printf("pool event: %s -> %" PRIu64 "\n", queued->source->name, queued->doubled);

    cl_free(allocator, queued, sizeof(*queued), CL_ALIGNOF_TYPE(event));
    return true;
}

static void preview_name_stream(const item_array *items)
{
    unsigned char storage[32];
    char out[17];
    cl_ring_buffer ring;
    size_t read;

    if (!items || items->size == 0u) {
        return;
    }

    cl_ring_buffer_init(&ring, storage, sizeof(storage));
    (void)cl_ring_buffer_write(
        &ring,
        items->data[0].name,
        cl_strlen(items->data[0].name));
    (void)cl_ring_buffer_write(&ring, "\n", 1u);

    read = cl_ring_buffer_read(&ring, out, sizeof(out) - 1u);
    out[read] = '\0';
    printf("ring preview: %s", out);
}

int main(void)
{
    static unsigned char list_storage[4096];
    static unsigned char arena_storage[512];
    static unsigned char pool_storage[256];
    const char *input = " apples: 4, oranges: 2, empty: 0, pears: 7 ";
    cl_free_list free_list;
    cl_debug_allocator debug;
    cl_allocator backing;
    cl_allocator tracked;
    cl_arena scratch;
    cl_pool pool;
    cl_allocator pool_allocator;
    cl_hash_table name_index;
    cl_file_data input_file;
    item_array items;
    char input_path[128];
    char report_path[128];
    void *found = NULL;

    if (!cl_free_list_init(&free_list, list_storage, sizeof(list_storage))) {
        fputs("free-list init failed\n", stderr);
        return 1;
    }

    backing = cl_free_list_allocator(&free_list);
    cl_debug_allocator_init(&debug, backing);
    tracked = cl_debug_allocator_view(&debug);
    item_array_init(&items, &tracked);

    make_example_path(input_path, sizeof(input_path));
    (void)unlink(input_path);
    if (!cl_file_write_all(input_path, input, cl_strlen(input))) {
        fputs("input write failed\n", stderr);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }
    if (!cl_file_read_all(input_path, &tracked, &input_file)) {
        fputs("input read failed\n", stderr);
        (void)unlink(input_path);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }

    printf("input file: %zu bytes\n", input_file.size);
    if (!parse_items(
            cl_sv_from_parts((const char *)input_file.data, input_file.size),
            &items)) {
        fputs("parse failed\n", stderr);
        cl_file_data_free(&input_file);
        (void)unlink(input_path);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }
    cl_file_data_free(&input_file);
    (void)unlink(input_path);

    compact_items(&items);
    if (items.size >= 2u &&
        cl_strcmp(items.data[0].name, items.data[1].name) < 0) {
        puts("inventory, sorted by input order:");
    } else {
        puts("inventory:");
    }

    cl_arena_init(&scratch, arena_storage, sizeof(arena_storage));
    print_report(&items, &scratch);
    printf("total      %" PRIu64 "\n", total_count(&items));

    cl_hash_table_init(&name_index, &tracked);
    if (!index_items(&items, &name_index)) {
        fputs("index failed\n", stderr);
        cl_hash_table_free(&name_index);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }
    if (cl_hash_table_get_cstr(&name_index, "pears", &found)) {
        const item *pears = (const item *)found;
        printf("lookup     %s=%" PRIu64 "\n", pears->name, pears->count);
    }

    if (!cl_pool_init(
            &pool,
            pool_storage,
            sizeof(pool_storage),
            sizeof(event),
            CL_ALIGNOF_TYPE(event))) {
        fputs("pool init failed\n", stderr);
        cl_hash_table_free(&name_index);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }

    pool_allocator = cl_pool_allocator(&pool);
    if (items.size != 0u && !queue_event(&pool_allocator, &items.data[0])) {
        fputs("event allocation failed\n", stderr);
        cl_hash_table_free(&name_index);
        item_array_free(&items);
        cl_debug_allocator_release(&debug);
        return 1;
    }
    preview_name_stream(&items);
    if (cl_path_join("/tmp/clibs_overview", "../clibs_report.txt", report_path, sizeof(report_path), NULL)) {
        cl_path_view report_name = cl_path_basename(report_path);

        printf("path: %.*s\n", (int)report_name.size, report_name.data);
    }

    printf(
        "debug: allocations=%zu frees=%zu peak=%zu live=%zu\n",
        debug.allocation_count,
        debug.free_count,
        debug.peak_bytes,
        debug.live_bytes);
    printf(
        "pool: blocks=%zu free=%zu\n",
        cl_pool_block_count(&pool),
        cl_pool_free_count(&pool));
    printf(
        "free-list: used=%zu free=%zu\n",
        cl_free_list_used_bytes(&free_list),
        cl_free_list_free_bytes(&free_list));

    cl_hash_table_free(&name_index);
    item_array_free(&items);
    printf("after free: debug live=%zu\n", debug.live_bytes);
    cl_debug_allocator_release(&debug);
    return 0;
}
