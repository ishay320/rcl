#include "graph.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "test_suite.h"

void test_graph_initialization(void) {
    struct graph* g = graph_init();
    assert(g != NULL);
    assert(g->nodes != NULL);
    assert(g->edges != NULL);
    assert(g->inc_id == 0);

    graph_free(g);
}

void test_push_node(void) {
    struct graph* g = graph_init();
    int* p          = malloc(sizeof(*p));

    size_t id1 = graph_node_push(g, p);
    size_t id2 = graph_node_push(g, p);

    assert(array_len(g->nodes) == 2);
    assert(g->nodes[0].id == id1);
    assert(g->nodes[1].id == id2);

    assert(g->nodes[0].data == p);
    assert(g->nodes[1].data == p);

    graph_free(g);
    free(p);
}

void test_connect_nodes(void) {
    struct graph* g = graph_init();
    int* p          = malloc(sizeof(*p));

    size_t id1 = graph_node_push(g, p);
    size_t id2 = graph_node_push(g, p);

    assert(graph_eadge_create(g, id1, id2) == true);
    assert(array_len(g->edges) == 1);

    struct edge e = g->edges[0];
    assert(e.id_node_from == id1);
    assert(e.id_node_to == id2);

    assert(graph_eadge_create(g, id1, 999) == false);

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

    assert(n1 != NULL);
    assert(n2 != NULL);

    assert(n1->id == id1);
    assert(n2->id == id2);

    assert(n1->data == p1);
    assert(n2->data == p2);

    assert(graph_node_get(g, 999) == NULL);

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

    assert(found != NULL);
    assert(*((int*)found->data) == 42);

    graph_free(g);
    free(p1);
    free(p2);
}

void test_graph_edge_creation_memory_safety(void) {
    struct graph* g = graph_init();
    assert(graph_eadge_create(g, 1, 2) == false);
    assert(array_len(g->edges) == 0);

    graph_free(g);
}

void test_graph_node_edges_get(void) {
    struct graph* g = graph_init();
    int* foo        = NULL;
    size_t node_a   = graph_node_push(g, foo);
    size_t node_b   = graph_node_push(g, foo);

    size_t len;
    struct edge* e = graph_node_edges_get(g, node_a, &len);
    assert(len == 0);
    free(e);

    assert(graph_eadge_create(g, node_a, node_b) == true);
    e = graph_node_edges_get(g, node_a, &len);
    assert(len == 1);
    assert(e[0].id_node_to == node_b);
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
