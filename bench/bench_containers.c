/*
 * bench_containers.c
 * Purpose: Comparative benchmark for clibs container operations.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "cl_alloc.h"
#include "cl_array.h"
#include "cl_bench.h"
#include "cl_bitset.h"
#include "cl_buffer.h"
#include "cl_hash.h"
#include "cl_heap.h"
#include "cl_list.h"
#include "cl_map.h"
#include "cl_priority_queue.h"
#include "cl_queue.h"
#include "cl_set.h"

#include <stdint.h>
#include <stdio.h>

#define CL_BENCH_CONTAINER_COUNT 4096u
#define CL_BENCH_CONTAINER_ROUNDS 1000u
#define CL_BENCH_LOOKUP_ROUNDS 4000u
#define CL_BENCH_TREE_STORAGE_SIZE (1024u * 1024u)
#define CL_BENCH_HASH_STORAGE_SIZE (1024u * 1024u)

typedef struct bench_list_item {
    uint64_t value;
    cl_list_node node;
} bench_list_item;

CL_ARRAY_DEFINE(bench_u64_array, uint64_t)

static uint64_t cl_bench_keys[CL_BENCH_CONTAINER_COUNT];
static uintptr_t cl_bench_values[CL_BENCH_CONTAINER_COUNT];
static bool cl_bench_data_ready;

static void cl_bench_prepare_data(void)
{
    uint64_t state = UINT64_C(0x9e3779b97f4a7c15);
    size_t i;

    if (cl_bench_data_ready) {
        return;
    }

    for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
        state = (state * UINT64_C(6364136223846793005)) + 1u;
        cl_bench_keys[i] = state ^ ((uint64_t)i << 32u);
        cl_bench_values[i] = (uintptr_t)(i + 1u);
    }
    cl_bench_data_ready = true;
}

static int cl_bench_compare_u64_max(
    const void *left,
    const void *right,
    void *user)
{
    const uint64_t *a = (const uint64_t *)left;
    const uint64_t *b = (const uint64_t *)right;

    (void)user;
    if (*a > *b) {
        return 1;
    }
    if (*a < *b) {
        return -1;
    }
    return 0;
}

static cl_bench_result cl_bench_array_reserved_push_pop(void)
{
    cl_allocator allocator = cl_system_allocator();
    bench_u64_array array;
    double start;
    size_t round;
    size_t i;

    bench_u64_array_init(&array, &allocator);
    if (!bench_u64_array_reserve(&array, CL_BENCH_CONTAINER_COUNT)) {
        return cl_bench_report("array reserved push/pop u64", 0u, 0.0);
    }

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)bench_u64_array_push(&array, cl_bench_keys[i]);
        }
        while (array.size != 0u) {
            uint64_t value;

            (void)bench_u64_array_pop(&array, &value);
            cl_bench_use_ptr((const void *)(uintptr_t)value);
        }
    }

    bench_u64_array_free(&array);
    return cl_bench_report(
        "array reserved push/pop u64",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_queue_push_pop(void)
{
    uint64_t storage[CL_BENCH_CONTAINER_COUNT];
    cl_queue queue;
    double start;
    size_t round;
    size_t i;

    cl_queue_init(&queue, storage, sizeof(storage), sizeof(storage[0]));

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_queue_push(&queue, &cl_bench_keys[i]);
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            uint64_t value;

            (void)cl_queue_pop(&queue, &value);
            cl_bench_use_ptr((const void *)(uintptr_t)value);
        }
    }

    return cl_bench_report(
        "queue fixed ring push/pop u64",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_ring_buffer_push_pop(void)
{
    unsigned char storage[CL_BENCH_CONTAINER_COUNT];
    cl_ring_buffer ring;
    double start;
    size_t round;
    size_t i;

    cl_ring_buffer_init(&ring, storage, sizeof(storage));

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_ring_buffer_push(&ring, (unsigned char)i);
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            unsigned char value;

            (void)cl_ring_buffer_pop(&ring, &value);
            cl_bench_use_ptr((const void *)(uintptr_t)value);
        }
    }

    return cl_bench_report(
        "ring buffer push/pop byte",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_list_push_pop(void)
{
    bench_list_item items[CL_BENCH_CONTAINER_COUNT];
    cl_list list;
    double start;
    size_t round;
    size_t i;

    for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
        items[i].value = cl_bench_keys[i];
        cl_list_node_init(&items[i].node);
    }
    cl_list_init(&list);

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_list_push_back(&list, &items[i].node);
        }
        while (!cl_list_is_empty(&list)) {
            bench_list_item *item = CL_CONTAINER_OF(
                cl_list_pop_front(&list),
                bench_list_item,
                node);

            cl_bench_use_ptr((const void *)(uintptr_t)item->value);
        }
    }

    return cl_bench_report(
        "intrusive list push_back/pop_front",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_heap_make_pop(void)
{
    uint64_t storage[CL_BENCH_CONTAINER_COUNT];
    double start;
    size_t round;
    size_t i;

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        size_t count = CL_BENCH_CONTAINER_COUNT;

        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            storage[i] = cl_bench_keys[i];
        }
        (void)cl_heap_make(
            storage,
            count,
            sizeof(storage[0]),
            cl_bench_compare_u64_max,
            NULL);
        while (count != 0u) {
            (void)cl_heap_pop(
                storage,
                count,
                sizeof(storage[0]),
                cl_bench_compare_u64_max,
                NULL);
            --count;
            cl_bench_use_ptr((const void *)(uintptr_t)storage[count]);
        }
    }

    return cl_bench_report(
        "heap make + pop all u64",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_priority_queue_push_pop(void)
{
    uint64_t storage[CL_BENCH_CONTAINER_COUNT];
    cl_priority_queue queue;
    double start;
    size_t round;
    size_t i;

    cl_priority_queue_init(
        &queue,
        storage,
        sizeof(storage),
        sizeof(storage[0]),
        cl_bench_compare_u64_max,
        NULL);

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_priority_queue_push(&queue, &cl_bench_keys[i]);
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            uint64_t value;

            (void)cl_priority_queue_pop(&queue, &value);
            cl_bench_use_ptr((const void *)(uintptr_t)value);
        }
    }

    return cl_bench_report(
        "priority queue push/pop u64",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_bitset_set_count(void)
{
    size_t storage[(CL_BENCH_CONTAINER_COUNT / (sizeof(size_t) * 8u)) + 1u];
    cl_bitset bitset;
    double start;
    size_t round;
    size_t i;

    if (!cl_bitset_init_with_storage(
            &bitset,
            storage,
            sizeof(storage),
            CL_BENCH_CONTAINER_COUNT)) {
        return cl_bench_report("bitset set/count/clear", 0u, 0.0);
    }

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; i += 2u) {
            (void)cl_bitset_set(&bitset, i);
        }
        cl_bench_use_ptr((const void *)(uintptr_t)cl_bitset_count(&bitset));
        cl_bitset_clear_all(&bitset);
    }

    return cl_bench_report(
        "bitset set half/count/clear",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_hash_put_get_clear(void)
{
    static unsigned char storage[CL_BENCH_HASH_STORAGE_SIZE];
    cl_free_list free_list;
    cl_allocator allocator;
    cl_hash_table table;
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&free_list, storage, sizeof(storage))) {
        return cl_bench_report("hash put/get/clear u64 keys", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&free_list);
    cl_hash_table_init(&table, &allocator);

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_hash_table_put(
                &table,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &cl_bench_values[i]);
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            void *value = NULL;

            (void)cl_hash_table_get(
                &table,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &value);
            cl_bench_use_ptr(value);
        }
        cl_hash_table_clear(&table);
    }
    cl_hash_table_free(&table);

    return cl_bench_report(
        "hash put/get/clear u64 keys",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_set_insert_contains_clear(void)
{
    static unsigned char storage[CL_BENCH_HASH_STORAGE_SIZE];
    cl_free_list free_list;
    cl_allocator allocator;
    cl_set set;
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&free_list, storage, sizeof(storage))) {
        return cl_bench_report("set insert/contains/clear u64", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&free_list);
    cl_set_init(&set, &allocator);

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_set_insert(
                &set,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]));
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            if (cl_set_contains(
                    &set,
                    &cl_bench_keys[i],
                    sizeof(cl_bench_keys[i]))) {
                cl_bench_use_ptr(&cl_bench_keys[i]);
            }
        }
        cl_set_clear(&set);
    }
    cl_set_free(&set);

    return cl_bench_report(
        "set insert/contains/clear u64",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_map_put_get_clear(void)
{
    static unsigned char storage[CL_BENCH_TREE_STORAGE_SIZE];
    cl_free_list free_list;
    cl_allocator allocator;
    cl_map map;
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&free_list, storage, sizeof(storage))) {
        return cl_bench_report("map put/get/clear u64 keys", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&free_list);
    cl_map_init(&map, &allocator);

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_CONTAINER_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            (void)cl_map_put(
                &map,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &cl_bench_values[i]);
        }
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            void *value = NULL;

            (void)cl_map_get(
                &map,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &value);
            cl_bench_use_ptr(value);
        }
        cl_map_clear(&map);
    }
    cl_map_free(&map);

    return cl_bench_report(
        "map put/get/clear u64 keys",
        CL_BENCH_CONTAINER_ROUNDS * CL_BENCH_CONTAINER_COUNT * 2u,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_hash_hot_get(void)
{
    static unsigned char storage[CL_BENCH_HASH_STORAGE_SIZE];
    cl_free_list free_list;
    cl_allocator allocator;
    cl_hash_table table;
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&free_list, storage, sizeof(storage))) {
        return cl_bench_report("hash hot get u64 keys", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&free_list);
    cl_hash_table_init(&table, &allocator);
    for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
        (void)cl_hash_table_put(
            &table,
            &cl_bench_keys[i],
            sizeof(cl_bench_keys[i]),
            &cl_bench_values[i]);
    }

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_LOOKUP_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            void *value = NULL;

            (void)cl_hash_table_get(
                &table,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &value);
            cl_bench_use_ptr(value);
        }
    }
    cl_hash_table_free(&table);

    return cl_bench_report(
        "hash hot get u64 keys",
        CL_BENCH_LOOKUP_ROUNDS * CL_BENCH_CONTAINER_COUNT,
        cl_bench_now_seconds() - start);
}

static cl_bench_result cl_bench_map_hot_get(void)
{
    static unsigned char storage[CL_BENCH_TREE_STORAGE_SIZE];
    cl_free_list free_list;
    cl_allocator allocator;
    cl_map map;
    double start;
    size_t round;
    size_t i;

    if (!cl_free_list_init(&free_list, storage, sizeof(storage))) {
        return cl_bench_report("map hot get u64 keys", 0u, 0.0);
    }
    allocator = cl_free_list_allocator(&free_list);
    cl_map_init(&map, &allocator);
    for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
        (void)cl_map_put(
            &map,
            &cl_bench_keys[i],
            sizeof(cl_bench_keys[i]),
            &cl_bench_values[i]);
    }

    start = cl_bench_now_seconds();
    for (round = 0u; round < CL_BENCH_LOOKUP_ROUNDS; ++round) {
        for (i = 0u; i < CL_BENCH_CONTAINER_COUNT; ++i) {
            void *value = NULL;

            (void)cl_map_get(
                &map,
                &cl_bench_keys[i],
                sizeof(cl_bench_keys[i]),
                &value);
            cl_bench_use_ptr(value);
        }
    }
    cl_map_free(&map);

    return cl_bench_report(
        "map hot get u64 keys",
        CL_BENCH_LOOKUP_ROUNDS * CL_BENCH_CONTAINER_COUNT,
        cl_bench_now_seconds() - start);
}

int main(void)
{
    cl_bench_result array_ops;
    cl_bench_result queue_ops;
    cl_bench_result ring_ops;
    cl_bench_result list_ops;
    cl_bench_result heap_ops;
    cl_bench_result priority_queue_ops;
    cl_bench_result bitset_ops;
    cl_bench_result hash_mutation;
    cl_bench_result set_mutation;
    cl_bench_result map_mutation;
    cl_bench_result hash_hot;
    cl_bench_result map_hot;

    cl_bench_prepare_data();
    cl_bench_print_table_header(
        "cl_container benchmark",
        "note: hash/set/map mutation rows include backing allocator work");

    array_ops = cl_bench_array_reserved_push_pop();
    queue_ops = cl_bench_queue_push_pop();
    ring_ops = cl_bench_ring_buffer_push_pop();
    list_ops = cl_bench_list_push_pop();
    heap_ops = cl_bench_heap_make_pop();
    priority_queue_ops = cl_bench_priority_queue_push_pop();
    bitset_ops = cl_bench_bitset_set_count();
    hash_mutation = cl_bench_hash_put_get_clear();
    set_mutation = cl_bench_set_insert_contains_clear();
    map_mutation = cl_bench_map_put_get_clear();
    hash_hot = cl_bench_hash_hot_get();
    map_hot = cl_bench_map_hot_get();

    putchar('\n');
    puts("interpretation");
    cl_bench_print_ratio("queue push/pop cost", &queue_ops, &array_ops);
    cl_bench_print_ratio("ring byte push/pop cost", &ring_ops, &array_ops);
    cl_bench_print_ratio("list push/pop cost", &list_ops, &array_ops);
    cl_bench_print_ratio("priority queue op vs heap pop workload",
                         &priority_queue_ops, &heap_ops);
    cl_bench_print_ratio("hash mutation cost vs hot get", &hash_mutation,
                         &hash_hot);
    cl_bench_print_ratio("set mutation cost vs hash mutation", &set_mutation,
                         &hash_mutation);
    cl_bench_print_ratio("map mutation cost vs hash mutation", &map_mutation,
                         &hash_mutation);
    cl_bench_print_ratio("map hot get cost", &map_hot, &hash_hot);
    cl_bench_print_speedup("bitset bulk bit operations", &bitset_ops,
                           &array_ops);

    putchar('\n');
    printf("sink: %lu\n", cl_bench_sink_value());
    return 0;
}
