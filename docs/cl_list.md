<!--
docs/cl_list.md
Purpose: Intrusive linked list library documentation.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
-->

# cl_list

`cl_list` provides allocation-free intrusive doubly linked lists. The list owns
no nodes; callers embed `cl_list_node` inside their own objects and control the
object lifetime.

## Example

```c
typedef struct job {
    int id;
    cl_list_node link;
} job;

cl_list ready;
job first;
job *front;

cl_list_init(&ready);
cl_list_node_init(&first.link);
first.id = 7;

if (cl_list_push_back(&ready, &first.link)) {
    front = CL_CONTAINER_OF(cl_list_front(&ready), job, link);
}
```

## Contracts

- Lists and nodes must be initialized before use.
- A node can be linked into at most one list at a time.
- Removing, popping, or clearing a list unlinks nodes but does not free the
  containing objects.
- Containing objects must outlive any list entry that references their embedded
  node.
- `cl_list_insert_before`, `cl_list_insert_after`, and `cl_list_remove` verify
  in O(1) time that the position or node is currently in the target list.

## Operations

- `cl_list_push_front` and `cl_list_push_back` insert initialized unlinked nodes.
- `cl_list_insert_before` and `cl_list_insert_after` insert relative to an
  existing node.
- `cl_list_pop_front` and `cl_list_pop_back` unlink and return end nodes.
- `cl_list_remove` unlinks a specific node from a list.
- `cl_list_clear` unlinks every node while keeping the list usable.
- `cl_list_front`, `cl_list_back`, `cl_list_next`, and `cl_list_prev` support
  explicit iteration.
- `CL_CONTAINER_OF` recovers the containing object from an embedded node pointer.

## Safety Properties

- The implementation performs no allocation and cannot fail due to allocator
  state.
- Insertions reject already-linked nodes, which prevents accidentally linking
  the same node into multiple lists.
- Removal and relative insertion check target-list membership before mutating
  links.
- Unlinked nodes are reset to null links, making stale double removes fail
  predictably.

## Portability

The implementation targets POSIX.1-2008 compatible C99. It uses only standard C
pointer operations and `offsetof`.
