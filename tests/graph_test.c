#include "graph.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "test_suite.h"

void test_graph_initialization(void) {
    struct graph* g = graph_init();
    ASSERT_TRUE(g != NULL);
    ASSERT_TRUE(g->nodes != NULL);
    ASSERT_TRUE(g->edges != NULL);
    ASSERT_CMP(g->inc_id, (size_t)0);

    graph_free(g);
}

void test_push_node(void) {
    struct graph* g = graph_init();
    int* p          = malloc(sizeof(*p));

    size_t id1 = graph_node_push(g, p);
    size_t id2 = graph_node_push(g, p);

    ASSERT_CMP(array_len(g->nodes), (size_t)2);
    ASSERT_CMP(g->nodes[0].id, id1);
    ASSERT_CMP(g->nodes[1].id, id2);

    ASSERT_CMP(g->nodes[0].data, p);
    ASSERT_CMP(g->nodes[1].data, p);

    graph_free(g);
    free(p);
}

void test_connect_nodes(void) {
    struct graph* g = graph_init();
    int* p          = malloc(sizeof(*p));

    size_t id1 = graph_node_push(g, p);
    size_t id2 = graph_node_push(g, p);

    ASSERT_TRUE(graph_eadge_create(g, id1, id2));
    ASSERT_CMP(array_len(g->edges), (size_t)1);

    struct edge e = g->edges[0];
    ASSERT_CMP(e.id_node_from, id1);
    ASSERT_CMP(e.id_node_to, id2);

    ASSERT_FALSE(graph_eadge_create(g, id1, 999));

    graph_free(g);
    free(p);
}

void test_free_graph(void) {
    struct graph* g = graph_init();
    int* p          = malloc(sizeof(*p));

    graph_node_push(g, p);
    graph_free(g);
    free(p);
}

void test_graph_node_get(void) {
    struct graph* g = graph_init();
    int* p1         = malloc(sizeof(*p1));
    int* p2         = malloc(sizeof(*p2));

    size_t id1 = graph_node_push(g, p1);
    size_t id2 = graph_node_push(g, p2);

    struct node_i* n1 = graph_node_get(g, id1);
    struct node_i* n2 = graph_node_get(g, id2);

    ASSERT_TRUE(n1 != NULL);
    ASSERT_TRUE(n2 != NULL);

    ASSERT_CMP(n1->id, id1);
    ASSERT_CMP(n2->id, id2);

    ASSERT_CMP(n1->data, p1);
    ASSERT_CMP(n2->data, p2);

    ASSERT_CMP(graph_node_get(g, 999), NULL);

    graph_free(g);
    free(p1);
    free(p2);
}

bool node_equals(struct node_i n) { return *((int*)n.data) == 42; }

void test_graph_node_find(void) {
    struct graph* g = graph_init();
    int* p1         = malloc(sizeof(*p1));
    int* p2         = malloc(sizeof(*p2));

    *p1 = 42;
    *p2 = 99;

    graph_node_push(g, p1);
    graph_node_push(g, p2);

    struct node_i* found = graph_node_find(g, node_equals);

    ASSERT_TRUE(found != NULL);
    ASSERT_CMP(*((int*)found->data), 42);

    graph_free(g);
    free(p1);
    free(p2);
}

void test_graph_edge_creation_memory_safety(void) {
    struct graph* g = graph_init();
    ASSERT_FALSE(graph_eadge_create(g, 1, 2));
    ASSERT_CMP(array_len(g->edges), (size_t)0);

    graph_free(g);
}

void test_graph_node_edges_get(void) {
    struct graph* g = graph_init();
    int* foo        = NULL;
    size_t node_a   = graph_node_push(g, foo);
    size_t node_b   = graph_node_push(g, foo);

    size_t len;
    struct edge* e = graph_node_edges_get(g, node_a, &len);
    ASSERT_CMP(len, (size_t)0);
    free(e);

    ASSERT_TRUE(graph_eadge_create(g, node_a, node_b));
    e = graph_node_edges_get(g, node_a, &len);
    ASSERT_CMP(len, (size_t)1);
    ASSERT_CMP(e[0].id_node_to, node_b);
    free(e);

    graph_free(g);
}

int main(void) {
    START_SUITE;
    RUN(test_graph_initialization);
    RUN(test_push_node);
    RUN(test_connect_nodes);
    RUN(test_free_graph);
    RUN(test_graph_node_get);
    RUN(test_graph_node_find);
    RUN(test_graph_edge_creation_memory_safety);
    RUN(test_graph_node_edges_get);
    END_SUITE;
    return 0;
}
