/*
 * cl_map.c
 * Purpose: Implement ordered non-owning byte-key maps.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_map.h"

#include <stdint.h>
#include <string.h>

typedef struct cl_map_node {
    struct cl_map_node *left;
    struct cl_map_node *right;
    struct cl_map_node *parent;
    const void *key;
    size_t key_size;
    void *value;
    int height;
} cl_map_node;

static size_t cl_map_node_align(void)
{
    typedef struct cl_map_node_align_probe {
        char c;
        cl_map_node node;
    } cl_map_node_align_probe;

    return offsetof(cl_map_node_align_probe, node);
}

static int cl_map_height(const cl_map_node *node)
{
    return node ? node->height : 0;
}

static void cl_map_update_height(cl_map_node *node)
{
    int left_height;
    int right_height;

    if (!node) {
        return;
    }

    left_height = cl_map_height(node->left);
    right_height = cl_map_height(node->right);
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

static int cl_map_balance_factor(const cl_map_node *node)
{
    return node ? cl_map_height(node->left) - cl_map_height(node->right) : 0;
}

static int cl_map_key_compare(
    const void *left,
    size_t left_size,
    const void *right,
    size_t right_size)
{
    size_t shared = left_size < right_size ? left_size : right_size;
    int cmp = 0;

    if (shared != 0u) {
        cmp = memcmp(left, right, shared);
        if (cmp != 0) {
            return cmp < 0 ? -1 : 1;
        }
    }

    if (left_size < right_size) {
        return -1;
    }
    if (left_size > right_size) {
        return 1;
    }
    return 0;
}

static cl_map_node *cl_map_min_node(cl_map_node *node)
{
    if (!node) {
        return NULL;
    }

    while (node->left) {
        node = node->left;
    }
    return node;
}

static cl_map_node *cl_map_max_node(cl_map_node *node)
{
    if (!node) {
        return NULL;
    }

    while (node->right) {
        node = node->right;
    }
    return node;
}

static void cl_map_replace_child(
    cl_map *map,
    cl_map_node *parent,
    cl_map_node *old_child,
    cl_map_node *new_child)
{
    if (!parent) {
        map->root = new_child;
    } else if (parent->left == old_child) {
        parent->left = new_child;
    } else {
        parent->right = new_child;
    }

    if (new_child) {
        new_child->parent = parent;
    }
}

static cl_map_node *cl_map_rotate_left(cl_map *map, cl_map_node *node)
{
    cl_map_node *pivot = node->right;
    cl_map_node *parent = node->parent;

    node->right = pivot->left;
    if (pivot->left) {
        pivot->left->parent = node;
    }

    pivot->left = node;
    pivot->parent = parent;
    node->parent = pivot;
    cl_map_replace_child(map, parent, node, pivot);

    cl_map_update_height(node);
    cl_map_update_height(pivot);
    return pivot;
}

static cl_map_node *cl_map_rotate_right(cl_map *map, cl_map_node *node)
{
    cl_map_node *pivot = node->left;
    cl_map_node *parent = node->parent;

    node->left = pivot->right;
    if (pivot->right) {
        pivot->right->parent = node;
    }

    pivot->right = node;
    pivot->parent = parent;
    node->parent = pivot;
    cl_map_replace_child(map, parent, node, pivot);

    cl_map_update_height(node);
    cl_map_update_height(pivot);
    return pivot;
}

static cl_map_node *cl_map_rebalance_node(cl_map *map, cl_map_node *node)
{
    int balance;

    cl_map_update_height(node);
    balance = cl_map_balance_factor(node);

    if (balance > 1) {
        if (cl_map_balance_factor(node->left) < 0) {
            (void)cl_map_rotate_left(map, node->left);
        }
        return cl_map_rotate_right(map, node);
    }

    if (balance < -1) {
        if (cl_map_balance_factor(node->right) > 0) {
            (void)cl_map_rotate_right(map, node->right);
        }
        return cl_map_rotate_left(map, node);
    }

    return node;
}

static void cl_map_rebalance_from(cl_map *map, cl_map_node *node)
{
    while (node) {
        cl_map_node *next = node->parent;
        node = cl_map_rebalance_node(map, node);
        next = node->parent ? node->parent : next;
        node = next;
    }
}

static cl_map_node *cl_map_find_node(
    const cl_map *map,
    const void *key,
    size_t key_size,
    cl_map_node **out_parent,
    int *out_cmp)
{
    cl_map_node *node;
    cl_map_node *parent = NULL;
    int cmp = 0;

    if (out_parent) {
        *out_parent = NULL;
    }
    if (out_cmp) {
        *out_cmp = 0;
    }
    if (!map) {
        return NULL;
    }

    node = (cl_map_node *)map->root;
    while (node) {
        parent = node;
        cmp = cl_map_key_compare(key, key_size, node->key, node->key_size);
        if (cmp == 0) {
            if (out_parent) {
                *out_parent = parent;
            }
            if (out_cmp) {
                *out_cmp = cmp;
            }
            return node;
        }
        node = cmp < 0 ? node->left : node->right;
    }

    if (out_parent) {
        *out_parent = parent;
    }
    if (out_cmp) {
        *out_cmp = cmp;
    }
    return NULL;
}

static void cl_map_free_subtree(cl_map *map, cl_map_node *node)
{
    if (!node) {
        return;
    }

    cl_map_free_subtree(map, node->left);
    cl_map_free_subtree(map, node->right);
    cl_free(map->allocator, node, sizeof(*node), cl_map_node_align());
}

static cl_map_node *cl_map_successor(cl_map_node *node)
{
    cl_map_node *parent;

    if (!node) {
        return NULL;
    }
    if (node->right) {
        return cl_map_min_node(node->right);
    }

    parent = node->parent;
    while (parent && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static cl_map_node *cl_map_predecessor(cl_map_node *node)
{
    cl_map_node *parent;

    if (!node) {
        return NULL;
    }
    if (node->left) {
        return cl_map_max_node(node->left);
    }

    parent = node->parent;
    while (parent && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static void cl_map_remove_node(cl_map *map, cl_map_node *node)
{
    cl_map_node *rebalance_from;
    cl_map_node *child;

    if (node->left && node->right) {
        cl_map_node *successor = cl_map_min_node(node->right);
        node->key = successor->key;
        node->key_size = successor->key_size;
        node->value = successor->value;
        node = successor;
    }

    child = node->left ? node->left : node->right;
    rebalance_from = node->parent;
    cl_map_replace_child(map, node->parent, node, child);
    cl_free(map->allocator, node, sizeof(*node), cl_map_node_align());
    --map->size;

    if (rebalance_from) {
        cl_map_rebalance_from(map, rebalance_from);
    } else if (map->root) {
        cl_map_update_height((cl_map_node *)map->root);
    }
}

void cl_map_init(cl_map *map, cl_allocator *allocator)
{
    if (!map) {
        return;
    }

    map->root = NULL;
    map->size = 0u;
    map->allocator = allocator;
}

void cl_map_clear(cl_map *map)
{
    if (!map) {
        return;
    }

    cl_map_free_subtree(map, (cl_map_node *)map->root);
    map->root = NULL;
    map->size = 0u;
}

void cl_map_free(cl_map *map)
{
    if (!map) {
        return;
    }

    cl_map_clear(map);
    map->allocator = NULL;
}

size_t cl_map_size(const cl_map *map)
{
    return map ? map->size : 0u;
}

bool cl_map_is_empty(const cl_map *map)
{
    return !map || map->size == 0u;
}

bool cl_map_put(cl_map *map, const void *key, size_t key_size, void *value)
{
    cl_map_node *parent;
    cl_map_node *node;
    int cmp;

    if (!map || (key_size != 0u && !key)) {
        return false;
    }

    node = cl_map_find_node(map, key, key_size, &parent, &cmp);
    if (node) {
        node->value = value;
        return true;
    }

    node = cl_alloc(map->allocator, sizeof(*node), cl_map_node_align());
    if (!node) {
        return false;
    }

    node->left = NULL;
    node->right = NULL;
    node->parent = parent;
    node->key = key;
    node->key_size = key_size;
    node->value = value;
    node->height = 1;

    if (!parent) {
        map->root = node;
    } else if (cmp < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    ++map->size;
    cl_map_rebalance_from(map, parent ? parent : node);
    return true;
}

bool cl_map_get(
    const cl_map *map,
    const void *key,
    size_t key_size,
    void **out_value)
{
    cl_map_node *node;

    if (!map || (key_size != 0u && !key)) {
        return false;
    }

    node = cl_map_find_node(map, key, key_size, NULL, NULL);
    if (!node) {
        return false;
    }

    if (out_value) {
        *out_value = node->value;
    }
    return true;
}

bool cl_map_contains(const cl_map *map, const void *key, size_t key_size)
{
    return cl_map_get(map, key, key_size, NULL);
}

bool cl_map_remove(
    cl_map *map,
    const void *key,
    size_t key_size,
    void **out_value)
{
    cl_map_node *node;

    if (!map || (key_size != 0u && !key)) {
        return false;
    }

    node = cl_map_find_node(map, key, key_size, NULL, NULL);
    if (!node) {
        return false;
    }

    if (out_value) {
        *out_value = node->value;
    }
    cl_map_remove_node(map, node);
    return true;
}

bool cl_map_put_cstr(cl_map *map, const char *key, void *value)
{
    if (!key) {
        return false;
    }

    return cl_map_put(map, key, strlen(key), value);
}

bool cl_map_get_cstr(const cl_map *map, const char *key, void **out_value)
{
    if (!key) {
        return false;
    }

    return cl_map_get(map, key, strlen(key), out_value);
}

bool cl_map_contains_cstr(const cl_map *map, const char *key)
{
    return cl_map_get_cstr(map, key, NULL);
}

bool cl_map_remove_cstr(cl_map *map, const char *key, void **out_value)
{
    if (!key) {
        return false;
    }

    return cl_map_remove(map, key, strlen(key), out_value);
}

cl_map_iter cl_map_first(const cl_map *map)
{
    cl_map_iter iter;

    iter.node = map ? cl_map_min_node((cl_map_node *)map->root) : NULL;
    return iter;
}

cl_map_iter cl_map_last(const cl_map *map)
{
    cl_map_iter iter;

    iter.node = map ? cl_map_max_node((cl_map_node *)map->root) : NULL;
    return iter;
}

cl_map_iter cl_map_next(cl_map_iter iter)
{
    iter.node = cl_map_successor((cl_map_node *)iter.node);
    return iter;
}

cl_map_iter cl_map_prev(cl_map_iter iter)
{
    iter.node = cl_map_predecessor((cl_map_node *)iter.node);
    return iter;
}

bool cl_map_iter_is_valid(cl_map_iter iter)
{
    return iter.node != NULL;
}

const void *cl_map_iter_key(cl_map_iter iter, size_t *out_key_size)
{
    cl_map_node *node = (cl_map_node *)iter.node;

    if (!node) {
        if (out_key_size) {
            *out_key_size = 0u;
        }
        return NULL;
    }

    if (out_key_size) {
        *out_key_size = node->key_size;
    }
    return node->key;
}

void *cl_map_iter_value(cl_map_iter iter)
{
    cl_map_node *node = (cl_map_node *)iter.node;

    return node ? node->value : NULL;
}
