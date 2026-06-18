/*
 * cl_list.c
 * Purpose: Implement allocation-free intrusive doubly linked list helpers.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_list.h"

static bool cl_list_ready(const cl_list *list)
{
    return list && list->root.next && list->root.prev;
}

static bool cl_list_node_ready(const cl_list_node *node)
{
    return node && !node->prev && !node->next && !node->list;
}

static void cl_list_link_between(
    cl_list *list,
    cl_list_node *prev,
    cl_list_node *next,
    cl_list_node *node)
{
    node->prev = prev;
    node->next = next;
    node->list = list;
    prev->next = node;
    next->prev = node;
}

static cl_list_node *cl_list_unlink(cl_list_node *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
    node->list = NULL;
    return node;
}

void cl_list_init(cl_list *list)
{
    if (!list) {
        return;
    }

    list->root.prev = &list->root;
    list->root.next = &list->root;
    list->root.list = NULL;
    list->size = 0u;
}

void cl_list_node_init(cl_list_node *node)
{
    if (!node) {
        return;
    }

    node->prev = NULL;
    node->next = NULL;
    node->list = NULL;
}

size_t cl_list_size(const cl_list *list)
{
    return cl_list_ready(list) ? list->size : 0u;
}

bool cl_list_is_empty(const cl_list *list)
{
    return !cl_list_ready(list) || list->size == 0u;
}

bool cl_list_node_is_linked(const cl_list_node *node)
{
    return node && node->prev && node->next && node->list;
}

cl_list_node *cl_list_front(const cl_list *list)
{
    if (!cl_list_ready(list) || list->root.next == &list->root) {
        return NULL;
    }

    return list->root.next;
}

cl_list_node *cl_list_back(const cl_list *list)
{
    if (!cl_list_ready(list) || list->root.prev == &list->root) {
        return NULL;
    }

    return list->root.prev;
}

cl_list_node *cl_list_next(const cl_list *list, const cl_list_node *node)
{
    if (!cl_list_ready(list) || !node || !node->next ||
        node->next == &list->root) {
        return NULL;
    }

    return node->next;
}

cl_list_node *cl_list_prev(const cl_list *list, const cl_list_node *node)
{
    if (!cl_list_ready(list) || !node || !node->prev ||
        node->prev == &list->root) {
        return NULL;
    }

    return node->prev;
}

bool cl_list_push_front(cl_list *list, cl_list_node *node)
{
    if (!cl_list_ready(list) || !cl_list_node_ready(node)) {
        return false;
    }

    cl_list_link_between(list, &list->root, list->root.next, node);
    ++list->size;
    return true;
}

bool cl_list_push_back(cl_list *list, cl_list_node *node)
{
    if (!cl_list_ready(list) || !cl_list_node_ready(node)) {
        return false;
    }

    cl_list_link_between(list, list->root.prev, &list->root, node);
    ++list->size;
    return true;
}

bool cl_list_insert_before(
    cl_list *list,
    cl_list_node *position,
    cl_list_node *node)
{
    if (!cl_list_ready(list) || !cl_list_node_ready(node) ||
        !position || position->list != list) {
        return false;
    }

    cl_list_link_between(list, position->prev, position, node);
    ++list->size;
    return true;
}

bool cl_list_insert_after(
    cl_list *list,
    cl_list_node *position,
    cl_list_node *node)
{
    if (!cl_list_ready(list) || !cl_list_node_ready(node) ||
        !position || position->list != list) {
        return false;
    }

    cl_list_link_between(list, position, position->next, node);
    ++list->size;
    return true;
}

cl_list_node *cl_list_pop_front(cl_list *list)
{
    cl_list_node *node;

    if (!cl_list_ready(list) || list->size == 0u) {
        return NULL;
    }

    node = cl_list_unlink(list->root.next);
    --list->size;
    return node;
}

cl_list_node *cl_list_pop_back(cl_list *list)
{
    cl_list_node *node;

    if (!cl_list_ready(list) || list->size == 0u) {
        return NULL;
    }

    node = cl_list_unlink(list->root.prev);
    --list->size;
    return node;
}

bool cl_list_remove(cl_list *list, cl_list_node *node)
{
    if (!cl_list_ready(list) || !node || node->list != list) {
        return false;
    }

    (void)cl_list_unlink(node);
    --list->size;
    return true;
}

void cl_list_clear(cl_list *list)
{
    cl_list_node *node;
    cl_list_node *next;

    if (!cl_list_ready(list)) {
        return;
    }

    for (node = list->root.next; node != &list->root; node = next) {
        next = node->next;
        node->prev = NULL;
        node->next = NULL;
        node->list = NULL;
    }

    list->root.prev = &list->root;
    list->root.next = &list->root;
    list->size = 0u;
}
