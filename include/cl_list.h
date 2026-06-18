/*
 * cl_list.h
 * Purpose: Allocation-free intrusive doubly linked list helpers for clibs.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#ifndef CL_LIST_H
#define CL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cl_list cl_list;

typedef struct cl_list_node {
    struct cl_list_node *prev;
    struct cl_list_node *next;
    cl_list *list;
} cl_list_node;

struct cl_list {
    cl_list_node root;
    size_t size;
};

#define CL_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void cl_list_init(cl_list *list);
void cl_list_node_init(cl_list_node *node);

size_t cl_list_size(const cl_list *list);
bool cl_list_is_empty(const cl_list *list);
bool cl_list_node_is_linked(const cl_list_node *node);

cl_list_node *cl_list_front(const cl_list *list);
cl_list_node *cl_list_back(const cl_list *list);
cl_list_node *cl_list_next(const cl_list *list, const cl_list_node *node);
cl_list_node *cl_list_prev(const cl_list *list, const cl_list_node *node);

bool cl_list_push_front(cl_list *list, cl_list_node *node);
bool cl_list_push_back(cl_list *list, cl_list_node *node);
bool cl_list_insert_before(
    cl_list *list,
    cl_list_node *position,
    cl_list_node *node);
bool cl_list_insert_after(
    cl_list *list,
    cl_list_node *position,
    cl_list_node *node);

cl_list_node *cl_list_pop_front(cl_list *list);
cl_list_node *cl_list_pop_back(cl_list *list);
bool cl_list_remove(cl_list *list, cl_list_node *node);
void cl_list_clear(cl_list *list);

#ifdef __cplusplus
}
#endif

#endif
