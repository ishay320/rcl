#include "queue/queue_ll.h"

#include <limits.h>
#include <stddef.h>

#include "test_suite.h"

typedef struct node {
    struct node* next;
    int data;
} node;

TEST_CASE(test_qe_init, {
    node* q    = qe_init_header(node);
    queue* hdr = qe_get_header(q);
    ASSERT_TRUE(q != NULL);
    ASSERT_CMP(hdr->front, NULL);
    ASSERT_CMP(hdr->back, NULL);
    ASSERT_CMP(hdr->size, 0);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_push_single, {
    node* q = qe_init_header(node);
    int val = 7;
    ASSERT_TRUE(qe_push(q, val));
    queue* hdr = qe_get_header(q);
    ASSERT_TRUE(hdr->front != NULL);
    ASSERT_CMP(hdr->front, hdr->back);
    ASSERT_CMP(hdr->size, 1);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_pop_empty, {
    node* q = qe_init_header(node);
    int out = 0xDEAD;
    ASSERT_FALSE(qe_pop(q, out));
    ASSERT_CMP(out, 0xDEAD);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_push_pop_single, {
    node* q = qe_init_header(node);
    int val = 99;
    qe_push(q, val);
    int out    = 0;
    bool ok    = qe_pop(q, out);
    queue* hdr = qe_get_header(q);
    ASSERT_TRUE(ok);
    ASSERT_CMP(out, 99);
    ASSERT_CMP(hdr->front, NULL);
    ASSERT_CMP(hdr->back, NULL);
    ASSERT_CMP(hdr->size, 0);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_fifo_order, {
    node* q    = qe_init_header(node);
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(qe_push(q, vals[i]));
    int out = 0;
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(qe_pop(q, out));
        ASSERT_CMP(out, vals[i]);
    }
    ASSERT_TRUE(qe_is_empty(q));
    qe_destroy_header(q);
})

TEST_CASE(test_qe_pop_exhausted, {
    node* q = qe_init_header(node);
    int v   = 3;
    qe_push(q, v);
    qe_pop(q, v);
    ASSERT_FALSE(qe_pop(q, v));
    qe_destroy_header(q);
})

TEST_CASE(test_qe_interleaved, {
    node* q = qe_init_header(node);
    int a = 10, b = 20, c = 30, out = 0;
    qe_push(q, a);
    qe_push(q, b);
    qe_pop(q, out);
    ASSERT_CMP(out, 10);
    qe_push(q, c);
    qe_pop(q, out);
    ASSERT_CMP(out, 20);
    qe_pop(q, out);
    ASSERT_CMP(out, 30);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_boundary_values, {
    node* q    = qe_init_header(node);
    int vals[] = {0, -1, INT_MIN, INT_MAX};
    for (int i = 0; i < 4; i++) ASSERT_TRUE(qe_push(q, vals[i]));
    int out = 0;
    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE(qe_pop(q, out));
        ASSERT_CMP(out, vals[i]);
    }
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front_node, {
    node* q  = qe_init_header(node);
    node* fn = qe_front_node(q);
    ASSERT_CMP(fn, NULL);
    int val = 55;
    qe_push(q, val);
    fn = qe_front_node(q);
    ASSERT_TRUE(fn != NULL);
    ASSERT_CMP(fn->data, 55);
    ASSERT_CMP(qe_size(q), 1);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front_back_node_single, {
    node* q = qe_init_header(node);
    int val = 42;
    qe_push(q, val);
    ASSERT_CMP(qe_front_node(q), qe_back_node(q));
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_back_node, {
    node* q  = qe_init_header(node);
    node* bn = qe_back_node(q);
    ASSERT_CMP(bn, NULL);
    int a = 10, b = 20;
    qe_push(q, a);
    qe_push(q, b);
    bn = qe_back_node(q);
    ASSERT_TRUE(bn != NULL);
    ASSERT_CMP(bn->data, 20);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front_back_node_multi, {
    node* q = qe_init_header(node);
    int a = 1, b = 2;
    qe_push(q, a);
    qe_push(q, b);
    node* fn = qe_front_node(q);
    node* bn = qe_back_node(q);
    ASSERT_CMP(fn->data, 1);
    ASSERT_CMP(bn->data, 2);
    ASSERT_TRUE(fn != bn);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front, {
    node* q = qe_init_header(node);
    int out = 0;
    ASSERT_CMP(qe_front(q, out), NULL);
    int val = 77;
    qe_push(q, val);
    ASSERT_TRUE(qe_front(q, out) != NULL);
    ASSERT_CMP(out, 77);
    ASSERT_CMP(qe_size(q), 1);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_back, {
    node* q = qe_init_header(node);
    int out = 0;
    ASSERT_CMP(qe_back(q, out), NULL);
    int a = 10, b = 20;
    qe_push(q, a);
    qe_push(q, b);
    ASSERT_TRUE(qe_back(q, out) != NULL);
    ASSERT_CMP(out, 20);
    ASSERT_CMP(qe_size(q), 2);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_is_empty, {
    node* q = qe_init_header(node);
    ASSERT_TRUE(qe_is_empty(q));
    int v = 1;
    qe_push(q, v);
    ASSERT_FALSE(qe_is_empty(q));
    qe_pop(q, v);
    ASSERT_TRUE(qe_is_empty(q));
    qe_push(q, v);
    qe_clear(q);
    ASSERT_TRUE(qe_is_empty(q));
    qe_destroy_header(q);
})

TEST_CASE(test_qe_size, {
    node* q = qe_init_header(node);
    int v   = 0;
    ASSERT_CMP(qe_size(q), 0);
    qe_push(q, v);
    ASSERT_CMP(qe_size(q), 1);
    qe_push(q, v);
    ASSERT_CMP(qe_size(q), 2);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 1);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 0);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 0); /* no underflow */
    qe_destroy_header(q);
})

TEST_CASE(test_qe_clear, {
    node* q = qe_init_header(node);
    int v   = 0;
    qe_push(q, v);
    qe_push(q, v);
    qe_push(q, v);
    qe_clear(q);
    queue* hdr = qe_get_header(q);
    ASSERT_CMP(hdr->front, NULL);
    ASSERT_CMP(hdr->back, NULL);
    ASSERT_CMP(hdr->size, 0);
    v       = 77;
    bool ok = qe_push(q, v);
    int out = 0;
    qe_pop(q, out);
    ASSERT_TRUE(ok);
    ASSERT_CMP(out, 77);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_for_each, {
    node* q    = qe_init_header(node);
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) qe_push(q, vals[i]);
    int i = 0;
    qe_for_each(q, it) { ASSERT_CMP(it->data, vals[i++]); }
    ASSERT_CMP(i, 3);
    qe_clear(q);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_for_each_empty, {
    node* q   = qe_init_header(node);
    int count = 0;
    qe_for_each(q, it) {
        (void)it;
        count++;
    }
    ASSERT_CMP(count, 0);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_double_clear, {
    node* q = qe_init_header(node);
    int v   = 0;
    qe_push(q, v);
    qe_clear(q);
    qe_clear(q); /* should not crash or leak */
    ASSERT_TRUE(qe_is_empty(q));
    qe_destroy_header(q);
    qe_destroy_header(q); /* should not crash or double-free */
})

int main(void) {
    START_SUITE;
    RUN(test_qe_init);
    RUN(test_qe_push_single);
    RUN(test_qe_pop_empty);
    RUN(test_qe_push_pop_single);
    RUN(test_qe_fifo_order);
    RUN(test_qe_pop_exhausted);
    RUN(test_qe_interleaved);
    RUN(test_qe_boundary_values);
    RUN(test_qe_front_node);
    RUN(test_qe_front_back_node_single);
    RUN(test_qe_back_node);
    RUN(test_qe_front_back_node_multi);
    RUN(test_qe_front);
    RUN(test_qe_back);
    RUN(test_qe_is_empty);
    RUN(test_qe_size);
    RUN(test_qe_clear);
    RUN(test_qe_for_each);
    RUN(test_qe_for_each_empty);
    RUN(test_qe_double_clear);
    END_SUITE;
    return 0;
}
