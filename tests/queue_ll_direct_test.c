#include "queue/queue_ll_direct.h"

#include <limits.h>
#include <stdbool.h>

#include "test_suite.h"

typedef struct node {
    struct node* next;
    int data;
} node;

typedef struct {
    node* front;
    node* back;
    int size;
} queue;

void test_qd_init(void) {
    queue q = {0};
    ASSERT_CMP(q.front, NULL);
    ASSERT_CMP(q.back, NULL);
    ASSERT_CMP(q.size, 0);
}

void test_qd_push_single(void) {
    queue q = {0};
    int val = 7;
    ASSERT_TRUE(qd_push(&q, val));
    ASSERT_TRUE(q.front != NULL);
    ASSERT_CMP(q.front, q.back);
    ASSERT_CMP(q.size, 1);
    qd_clear(&q);
}

void test_qd_pop_empty(void) {
    queue q = {0};
    int out = 0xDEAD;
    ASSERT_FALSE(qd_pop(&q, out));
    ASSERT_CMP(out, 0xDEAD);
}

void test_qd_push_pop_single(void) {
    queue q = {0};
    int val = 99;
    qd_push(&q, val);
    int out = 0;
    bool ok = qd_pop(&q, out);
    ASSERT_TRUE(ok);
    ASSERT_CMP(out, 99);
    ASSERT_CMP(q.front, NULL);
    ASSERT_CMP(q.back, NULL);
    ASSERT_CMP(q.size, 0);
}

void test_qd_fifo_order(void) {
    queue q    = {0};
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(qd_push(&q, vals[i]));
    int out = 0;
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(qd_pop(&q, out));
        ASSERT_CMP(out, vals[i]);
    }
    ASSERT_TRUE(qd_is_empty(&q));
}

void test_qd_pop_exhausted(void) {
    queue q = {0};
    int v   = 3;
    qd_push(&q, v);
    qd_pop(&q, v);
    ASSERT_FALSE(qd_pop(&q, v));
}

void test_qd_interleaved(void) {
    queue q = {0};
    int a = 10, b = 20, c = 30, out = 0;
    qd_push(&q, a);
    qd_push(&q, b);
    qd_pop(&q, out);
    ASSERT_CMP(out, 10);
    qd_push(&q, c);
    qd_pop(&q, out);
    ASSERT_CMP(out, 20);
    qd_pop(&q, out);
    ASSERT_CMP(out, 30);
}

void test_qd_boundary_values(void) {
    queue q    = {0};
    int vals[] = {0, -1, INT_MIN, INT_MAX};
    for (int i = 0; i < 4; i++) ASSERT_TRUE(qd_push(&q, vals[i]));
    int out = 0;
    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE(qd_pop(&q, out));
        ASSERT_CMP(out, vals[i]);
    }
}

void test_qd_front_node(void) {
    queue q  = {0};
    node* fn = q.front;
    ASSERT_CMP(fn, NULL);
    int val = 55;
    qd_push(&q, val);
    fn = q.front;
    ASSERT_TRUE(fn != NULL);
    ASSERT_CMP(fn->data, 55);
    ASSERT_CMP(qd_size(&q), 1);
    qd_clear(&q);
}

void test_qd_front_back_node_single(void) {
    queue q = {0};
    int val = 42;
    qd_push(&q, val);
    ASSERT_CMP(q.front, q.back);
    qd_clear(&q);
}

void test_qd_back_node(void) {
    queue q  = {0};
    node* bn = q.back;
    ASSERT_CMP(bn, NULL);
    int a = 10, b = 20;
    qd_push(&q, a);
    qd_push(&q, b);
    bn = q.back;
    ASSERT_TRUE(bn != NULL);
    ASSERT_CMP(bn->data, 20);
    qd_clear(&q);
}

void test_qd_front_back_node_multi(void) {
    queue q = {0};
    int a = 1, b = 2;
    qd_push(&q, a);
    qd_push(&q, b);
    node* fn = q.front;
    node* bn = q.back;
    ASSERT_CMP(fn->data, 1);
    ASSERT_CMP(bn->data, 2);
    ASSERT_TRUE(fn != bn);
    qd_clear(&q);
}

void test_qd_front(void) {
    queue q = {0};
    int out = 0;
    ASSERT_CMP(qd_front(&q, out), NULL);
    int val = 77;
    qd_push(&q, val);
    ASSERT_TRUE(qd_front(&q, out) != NULL);
    ASSERT_CMP(out, 77);
    ASSERT_CMP(qd_size(&q), 1);
    qd_clear(&q);
}

void test_qd_back(void) {
    queue q = {0};
    int out = 0;
    ASSERT_CMP(qd_back(&q, out), NULL);
    int a = 10, b = 20;
    qd_push(&q, a);
    qd_push(&q, b);
    ASSERT_TRUE(qd_back(&q, out) != NULL);
    ASSERT_CMP(out, 20);
    ASSERT_CMP(qd_size(&q), 2);
    qd_clear(&q);
}

void test_qd_is_empty(void) {
    queue q = {0};
    ASSERT_TRUE(qd_is_empty(&q));
    int v = 1;
    qd_push(&q, v);
    ASSERT_FALSE(qd_is_empty(&q));
    qd_pop(&q, v);
    ASSERT_TRUE(qd_is_empty(&q));
    qd_push(&q, v);
    qd_clear(&q);
    ASSERT_TRUE(qd_is_empty(&q));
}

void test_qd_size(void) {
    queue q = {0};
    int v   = 0;
    ASSERT_CMP(qd_size(&q), 0);
    qd_push(&q, v);
    ASSERT_CMP(qd_size(&q), 1);
    qd_push(&q, v);
    ASSERT_CMP(qd_size(&q), 2);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 1);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 0);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 0); /* no underflow */
}

void test_qd_clear(void) {
    queue q = {0};
    int v   = 0;
    qd_push(&q, v);
    qd_push(&q, v);
    qd_push(&q, v);
    qd_clear(&q);
    ASSERT_CMP(q.front, NULL);
    ASSERT_CMP(q.back, NULL);
    ASSERT_CMP(q.size, 0);
    v       = 77;
    bool ok = qd_push(&q, v);
    int out = 0;
    qd_pop(&q, out);
    ASSERT_TRUE(ok);
    ASSERT_CMP(out, 77);
}

void test_qd_double_clear(void) {
    queue q = {0};
    int v   = 0;
    qd_push(&q, v);
    qd_clear(&q);
    qd_clear(&q); /* should not crash or leak */
    ASSERT_TRUE(qd_is_empty(&q));
}

int main(void) {
    START_SUITE;
    RUN(test_qd_init);
    RUN(test_qd_push_single);
    RUN(test_qd_pop_empty);
    RUN(test_qd_push_pop_single);
    RUN(test_qd_fifo_order);
    RUN(test_qd_pop_exhausted);
    RUN(test_qd_interleaved);
    RUN(test_qd_boundary_values);
    RUN(test_qd_front_node);
    RUN(test_qd_front_back_node_single);
    RUN(test_qd_back_node);
    RUN(test_qd_front_back_node_multi);
    RUN(test_qd_front);
    RUN(test_qd_back);
    RUN(test_qd_is_empty);
    RUN(test_qd_size);
    RUN(test_qd_clear);
    RUN(test_qd_double_clear);
    END_SUITE;
    return 0;
}
