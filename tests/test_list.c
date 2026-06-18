/*
 * test_list.c
 * Purpose: Safety and behavior tests for cl_list.
 * POSIX target: POSIX.1-2008 compatible C99.
 * Date modified: 2026-06-18.
 */

#include "cl_list.h"
#include "cl_test.h"

typedef struct list_item {
    int value;
    cl_list_node link;
} list_item;

static void init_item(list_item *item, int value)
{
    item->value = value;
    cl_list_node_init(&item->link);
}

static int test_push_pop_and_order(void)
{
    cl_list list;
    list_item a;
    list_item b;
    list_item c;
    cl_list_node *node;

    cl_list_init(&list);
    init_item(&a, 1);
    init_item(&b, 2);
    init_item(&c, 3);

    CL_TEST_CHECK(cl_list_is_empty(&list));
    CL_TEST_CHECK(cl_list_push_back(&list, &b.link));
    CL_TEST_CHECK(cl_list_push_front(&list, &a.link));
    CL_TEST_CHECK(cl_list_push_back(&list, &c.link));
    CL_TEST_CHECK(cl_list_size(&list) == 3u);

    node = cl_list_front(&list);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 1);
    node = cl_list_next(&list, node);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 2);
    node = cl_list_next(&list, node);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 3);
    CL_TEST_CHECK(cl_list_next(&list, node) == NULL);

    node = cl_list_pop_front(&list);
    CL_TEST_CHECK(node == &a.link);
    CL_TEST_CHECK(!cl_list_node_is_linked(&a.link));
    node = cl_list_pop_back(&list);
    CL_TEST_CHECK(node == &c.link);
    node = cl_list_pop_back(&list);
    CL_TEST_CHECK(node == &b.link);
    CL_TEST_CHECK(cl_list_pop_back(&list) == NULL);
    CL_TEST_CHECK(cl_list_is_empty(&list));

    return 0;
}

static int test_insert_before_after_and_remove(void)
{
    cl_list list;
    list_item a;
    list_item b;
    list_item c;
    list_item d;
    cl_list_node *node;

    cl_list_init(&list);
    init_item(&a, 1);
    init_item(&b, 2);
    init_item(&c, 3);
    init_item(&d, 4);

    CL_TEST_CHECK(cl_list_push_back(&list, &a.link));
    CL_TEST_CHECK(cl_list_insert_after(&list, &a.link, &c.link));
    CL_TEST_CHECK(cl_list_insert_before(&list, &c.link, &b.link));
    CL_TEST_CHECK(cl_list_insert_after(&list, &c.link, &d.link));
    CL_TEST_CHECK(cl_list_size(&list) == 4u);

    node = cl_list_back(&list);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 4);
    node = cl_list_prev(&list, node);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 3);
    node = cl_list_prev(&list, node);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 2);
    node = cl_list_prev(&list, node);
    CL_TEST_CHECK(CL_CONTAINER_OF(node, list_item, link)->value == 1);
    CL_TEST_CHECK(cl_list_prev(&list, node) == NULL);

    CL_TEST_CHECK(cl_list_remove(&list, &b.link));
    CL_TEST_CHECK(!cl_list_remove(&list, &b.link));
    CL_TEST_CHECK(!cl_list_node_is_linked(&b.link));
    CL_TEST_CHECK(cl_list_size(&list) == 3u);

    CL_TEST_CHECK(cl_list_remove(&list, &a.link));
    CL_TEST_CHECK(cl_list_front(&list) == &c.link);
    CL_TEST_CHECK(cl_list_remove(&list, &d.link));
    CL_TEST_CHECK(cl_list_back(&list) == &c.link);

    return 0;
}

static int test_rejects_uninitialized_and_double_linked_nodes(void)
{
    cl_list list;
    cl_list_node partially_linked;
    list_item a;
    list_item b;

    cl_list_init(&list);
    cl_list_node_init(&partially_linked);
    partially_linked.prev = &partially_linked;
    init_item(&a, 1);
    init_item(&b, 2);

    CL_TEST_CHECK(!cl_list_push_back(NULL, &a.link));
    CL_TEST_CHECK(!cl_list_push_back(&list, NULL));
    CL_TEST_CHECK(!cl_list_push_back(&list, &partially_linked));
    CL_TEST_CHECK(cl_list_push_back(&list, &a.link));
    CL_TEST_CHECK(!cl_list_push_back(&list, &a.link));
    CL_TEST_CHECK(!cl_list_node_is_linked(&list.root));
    CL_TEST_CHECK(!cl_list_remove(&list, &list.root));
    CL_TEST_CHECK(!cl_list_insert_before(&list, &b.link, &b.link));
    CL_TEST_CHECK(cl_list_size(&list) == 1u);

    return 0;
}

static int test_clear_unlinks_all_nodes(void)
{
    cl_list list;
    list_item a;
    list_item b;

    cl_list_init(&list);
    init_item(&a, 1);
    init_item(&b, 2);

    CL_TEST_CHECK(cl_list_push_back(&list, &a.link));
    CL_TEST_CHECK(cl_list_push_back(&list, &b.link));
    cl_list_clear(&list);
    CL_TEST_CHECK(cl_list_is_empty(&list));
    CL_TEST_CHECK(!cl_list_node_is_linked(&a.link));
    CL_TEST_CHECK(!cl_list_node_is_linked(&b.link));

    CL_TEST_CHECK(cl_list_push_front(&list, &b.link));
    CL_TEST_CHECK(cl_list_front(&list) == &b.link);

    return 0;
}

int main(void)
{
    const cl_test_case cases[] = {
        CL_TEST_CASE(test_push_pop_and_order),
        CL_TEST_CASE(test_insert_before_after_and_remove),
        CL_TEST_CASE(test_rejects_uninitialized_and_double_linked_nodes),
        CL_TEST_CASE(test_clear_unlinks_all_nodes)
    };

    return cl_test_run_all("cl_list", cases, sizeof(cases) / sizeof(cases[0]));
}
