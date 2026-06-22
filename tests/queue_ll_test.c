#include "queue/queue_ll.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>

#include "test_suite.h"

typedef struct node {
    struct node* next;
    int data;
} node;

void test_qe_init(void) {
    node* q    = qe_init_header(node);
    queue* hdr = qe_get_header(q);
    assert(q != NULL);
    assert(hdr->front == NULL);
    assert(hdr->back == NULL);
    assert(hdr->size == 0);
    qe_destroy_header(q);
}

void test_qe_push_single(void) {
    node* q = qe_init_header(node);
    int val = 7;
    assert(qe_push(q, val));
    queue* hdr = qe_get_header(q);
    assert(hdr->front != NULL);
    assert(hdr->front == hdr->back);
    assert(hdr->size == 1);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_pop_empty(void) {
    node* q = qe_init_header(node);
    int out = 0xDEAD;
    assert(!qe_pop(q, out));
    assert(out == 0xDEAD);
    qe_destroy_header(q);
}

void test_qe_push_pop_single(void) {
    node* q = qe_init_header(node);
    int val = 99;
    qe_push(q, val);
    int out    = 0;
    bool ok    = qe_pop(q, out);
    queue* hdr = qe_get_header(q);
    assert(ok == true);
    assert(out == 99);
    assert(hdr->front == NULL);
    assert(hdr->back == NULL);
    assert(hdr->size == 0);
    qe_destroy_header(q);
}

void test_qe_fifo_order(void) {
    node* q    = qe_init_header(node);
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) assert(qe_push(q, vals[i]));
    int out = 0;
    for (int i = 0; i < 5; i++) {
        assert(qe_pop(q, out));
        assert(out == vals[i]);
    }
    assert(qe_is_empty(q));
    qe_destroy_header(q);
}

void test_qe_pop_exhausted(void) {
    node* q = qe_init_header(node);
    int v   = 3;
    qe_push(q, v);
    qe_pop(q, v);
    assert(!qe_pop(q, v));
    qe_destroy_header(q);
}

void test_qe_interleaved(void) {
    node* q = qe_init_header(node);
    int a = 10, b = 20, c = 30, out = 0;
    qe_push(q, a);
    qe_push(q, b);
    qe_pop(q, out);
    assert(out == 10);
    qe_push(q, c);
    qe_pop(q, out);
    assert(out == 20);
    qe_pop(q, out);
    assert(out == 30);
    qe_destroy_header(q);
}

void test_qe_boundary_values(void) {
    node* q    = qe_init_header(node);
    int vals[] = {0, -1, INT_MIN, INT_MAX};
    for (int i = 0; i < 4; i++) assert(qe_push(q, vals[i]));
    int out = 0;
    for (int i = 0; i < 4; i++) {
        assert(qe_pop(q, out));
        assert(out == vals[i]);
    }
    qe_destroy_header(q);
}

void test_qe_front_node(void) {
    node* q  = qe_init_header(node);
    node* fn = qe_front_node(q);
    assert(fn == NULL);
    int val = 55;
    qe_push(q, val);
    fn = qe_front_node(q);
    assert(fn != NULL);
    assert(fn->data == 55);
    assert(qe_size(q) == 1);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_front_back_node_single(void) {
    node* q = qe_init_header(node);
    int val = 42;
    qe_push(q, val);
    assert(qe_front_node(q) == qe_back_node(q));
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_back_node(void) {
    node* q  = qe_init_header(node);
    node* bn = qe_back_node(q);
    assert(bn == NULL);
    int a = 10, b = 20;
    qe_push(q, a);
    qe_push(q, b);
    bn = qe_back_node(q);
    assert(bn != NULL);
    assert(bn->data == 20);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_front_back_node_multi(void) {
    node* q = qe_init_header(node);
    int a = 1, b = 2;
    qe_push(q, a);
    qe_push(q, b);
    node* fn = qe_front_node(q);
    node* bn = qe_back_node(q);
    assert(fn->data == 1);
    assert(bn->data == 2);
    assert(fn != bn);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_front(void) {
    node* q = qe_init_header(node);
    int out = 0;
    assert(qe_front(q, out) == NULL);
    int val = 77;
    qe_push(q, val);
    assert(qe_front(q, out) != NULL);
    assert(out == 77);
    assert(qe_size(q) == 1);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_back(void) {
    node* q = qe_init_header(node);
    int out = 0;
    assert(qe_back(q, out) == NULL);
    int a = 10, b = 20;
    qe_push(q, a);
    qe_push(q, b);
    assert(qe_back(q, out) != NULL);
    assert(out == 20);
    assert(qe_size(q) == 2);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_is_empty(void) {
    node* q = qe_init_header(node);
    assert(qe_is_empty(q));
    int v = 1;
    qe_push(q, v);
    assert(!qe_is_empty(q));
    qe_pop(q, v);
    assert(qe_is_empty(q));
    qe_push(q, v);
    qe_clear(q);
    assert(qe_is_empty(q));
    qe_destroy_header(q);
}

void test_qe_size(void) {
    node* q = qe_init_header(node);
    int v   = 0;
    assert(qe_size(q) == 0);
    qe_push(q, v);
    assert(qe_size(q) == 1);
    qe_push(q, v);
    assert(qe_size(q) == 2);
    qe_pop(q, v);
    assert(qe_size(q) == 1);
    qe_pop(q, v);
    assert(qe_size(q) == 0);
    qe_pop(q, v);
    assert(qe_size(q) == 0); /* no underflow */
    qe_destroy_header(q);
}

void test_qe_clear(void) {
    node* q = qe_init_header(node);
    int v   = 0;
    qe_push(q, v);
    qe_push(q, v);
    qe_push(q, v);
    qe_clear(q);
    queue* hdr = qe_get_header(q);
    assert(hdr->front == NULL);
    assert(hdr->back == NULL);
    assert(hdr->size == 0);
    v       = 77;
    bool ok = qe_push(q, v);
    int out = 0;
    qe_pop(q, out);
    assert(ok == true);
    assert(out == 77);
    qe_destroy_header(q);
}

void test_qe_for_each(void) {
    node* q    = qe_init_header(node);
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) qe_push(q, vals[i]);
    int i = 0;
    qe_for_each(q, it) { assert(it->data == vals[i++]); }
    assert(i == 3);
    qe_clear(q);
    qe_destroy_header(q);
}

void test_qe_for_each_empty(void) {
    node* q   = qe_init_header(node);
    int count = 0;
    qe_for_each(q, it) {
        (void)it;
        count++;
    }
    assert(count == 0);
    qe_destroy_header(q);
}

void test_qe_double_clear(void) {
    node* q = qe_init_header(node);
    int v   = 0;
    qe_push(q, v);
    qe_clear(q);
    qe_clear(q); /* should not crash or leak */
    assert(qe_is_empty(q));
    qe_destroy_header(q);
    qe_destroy_header(q); /* should not crash or double-free */
}

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
    SUMMERY;
    return 0;
}
