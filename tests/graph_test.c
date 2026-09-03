#include "graph.h"

#include <stddef.h>
#include <stdlib.h>

#include "test_suite.h"

TEST_CASE(test_node_push_and_get, {
    struct graph* g = graph_init();
    ASSERT_CMP(graph_nodes_count(g), (size_t)0, "new graph has no nodes");
    ASSERT_CMP(graph_edges_count(g), (size_t)0, "new graph has no edges");

    int a = 1, b = 2;
    size_t id_a = graph_node_push(g, &a);
    size_t id_b = graph_node_push(g, &b);
    ASSERT_TRUE(id_a != id_b, "every node gets a unique id");
    ASSERT_CMP(graph_nodes_count(g), (size_t)2);

    struct node_i* n = graph_node_get(g, id_a);
    ASSERT_TRUE(n != NULL);
    ASSERT_CMP(n->id, id_a);
    ASSERT_CMP(n->data, &a);

    n = graph_node_get(g, id_b);
    ASSERT_TRUE(n != NULL);
    ASSERT_CMP(n->data, &b);

    ASSERT_CMP(graph_node_get(g, 999), NULL, "unknown id");

    graph_free(g);
})

TEST_CASE(test_edge_create_between_existing_nodes, {
    struct graph* g = graph_init();
    int a           = 0;
    size_t id_a     = graph_node_push(g, &a);
    size_t id_b     = graph_node_push(g, &a);

    ASSERT_TRUE(graph_edge_create(g, id_a, id_b));
    ASSERT_CMP(graph_edges_count(g), (size_t)1);
    ASSERT_CMP(g->edges[0].id_node_from, id_a);
    ASSERT_CMP(g->edges[0].id_node_to, id_b);

    // edges are not deduplicated: an identical second edge is stored too
    ASSERT_TRUE(graph_edge_create(g, id_a, id_b));
    ASSERT_CMP(graph_edges_count(g), (size_t)2);

    graph_free(g);
})

TEST_CASE(test_edge_create_rejects_self_and_unknown_nodes, {
    struct graph* g = graph_init();
    ASSERT_FALSE(graph_edge_create(g, 0, 1), "no nodes exist yet");

    int a       = 0;
    size_t id_a = graph_node_push(g, &a);
    size_t id_b = graph_node_push(g, &a);
    ASSERT_FALSE(graph_edge_create(g, id_a, id_a), "self edge");
    ASSERT_FALSE(graph_edge_create(g, id_a, 999), "target id was never handed out");
    ASSERT_FALSE(graph_edge_create(g, 999, id_b), "source id was never handed out");
    ASSERT_FALSE(graph_edge_create(g, id_a, id_b + 1), "id just past the last node");
    ASSERT_CMP(graph_edges_count(g), (size_t)0, "no rejected edge was stored");

    graph_free(g);
})

static bool node_data_is_42(struct node_i n) { return *(int*)n.data == 42; }
static bool node_never_matches(struct node_i n) {
    (void)n;
    return false;
}

TEST_CASE(test_node_find_by_predicate, {
    struct graph* g = graph_init();
    int v1 = 42, v2 = 99;
    graph_node_push(g, &v1);
    graph_node_push(g, &v2);

    struct node_i* found = graph_node_find(g, node_data_is_42);
    ASSERT_TRUE(found != NULL);
    ASSERT_CMP(found->data, &v1);

    ASSERT_CMP(graph_node_find(g, node_never_matches), NULL, "no match");

    graph_free(g);
})

TEST_CASE(test_node_edges_get_returns_only_outgoing_edges, {
    struct graph* g = graph_init();
    size_t a        = graph_node_push(g, NULL);
    size_t b        = graph_node_push(g, NULL);
    size_t c        = graph_node_push(g, NULL);
    ASSERT_TRUE(graph_edge_create(g, a, b));
    ASSERT_TRUE(graph_edge_create(g, a, c));
    ASSERT_TRUE(graph_edge_create(g, b, c));

    size_t len         = 999;
    struct edge* edges = graph_node_edges_get(g, a, &len);
    ASSERT_CMP(len, (size_t)2);
    ASSERT_CMP(edges[0].id_node_from, a);
    ASSERT_CMP(edges[0].id_node_to, b);
    ASSERT_CMP(edges[1].id_node_to, c);
    free(edges);

    // edges are directed: c only has incoming edges, so nothing is returned
    edges = graph_node_edges_get(g, c, &len);
    ASSERT_CMP(len, (size_t)0);
    free(edges);

    graph_free(g);
})

int main(void) {
    START_SUITE;
    RUN(test_node_push_and_get);
    RUN(test_edge_create_between_existing_nodes);
    RUN(test_edge_create_rejects_self_and_unknown_nodes);
    RUN(test_node_find_by_predicate);
    RUN(test_node_edges_get_returns_only_outgoing_edges);
    END_SUITE;
}
