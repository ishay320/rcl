#include "queue/queue_ll.h"

#include <stddef.h>

#include "test_suite.h"

typedef struct node {
    struct node* next;
    int data;
} node;

TEST_CASE(test_qe_init, {
    node* q = qe_init_header(node);
    ASSERT_TRUE(q != NULL);
    queue* hdr = qe_get_header(q);
    ASSERT_CMP(hdr->front, NULL);
    ASSERT_CMP(hdr->back, NULL);
    ASSERT_CMP(hdr->size, 0);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_fifo_order, {
    node* q    = qe_init_header(node);
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(qe_push(q, vals[i]));
    ASSERT_FALSE(qe_is_empty(q));

    int out = 0;
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(qe_pop(q, out));
        ASSERT_CMP(out, vals[i]);
    }
    ASSERT_TRUE(qe_is_empty(q));
    qe_destroy_header(q);
})

TEST_CASE(test_qe_pop_empty_and_last, {
    node* q = qe_init_header(node);
    int out = 0xDEAD;
    ASSERT_FALSE(qe_pop(q, out));
    ASSERT_CMP(out, 0xDEAD, "out must stay untouched on empty pop");

    qe_push(q, 99);
    ASSERT_TRUE(qe_pop(q, out));
    ASSERT_CMP(out, 99);
    queue* hdr = qe_get_header(q);
    ASSERT_CMP(hdr->front, NULL, "popping the last node must reset front");
    ASSERT_CMP(hdr->back, NULL, "popping the last node must reset back");
    ASSERT_FALSE(qe_pop(q, out));

    /* queue is reusable after being drained */
    ASSERT_TRUE(qe_push(q, 7));
    ASSERT_TRUE(qe_pop(q, out));
    ASSERT_CMP(out, 7);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_interleaved, {
    node* q = qe_init_header(node);
    int out = 0;
    qe_push(q, 10);
    qe_push(q, 20);
    qe_pop(q, out);
    ASSERT_CMP(out, 10);
    qe_push(q, 30); /* push after a partial drain keeps order */
    qe_pop(q, out);
    ASSERT_CMP(out, 20);
    qe_pop(q, out);
    ASSERT_CMP(out, 30);
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front_back_node, {
    node* q = qe_init_header(node);
    ASSERT_CMP(qe_front_node(q), NULL);
    ASSERT_CMP(qe_back_node(q), NULL);

    qe_push(q, 1);
    node* fn = qe_front_node(q);
    ASSERT_TRUE(fn != NULL);
    ASSERT_CMP(fn->data, 1);
    ASSERT_CMP(qe_front_node(q), qe_back_node(q), "single node is both front and back");

    qe_push(q, 2);
    fn       = qe_front_node(q);
    node* bn = qe_back_node(q);
    ASSERT_CMP(fn->data, 1);
    ASSERT_CMP(bn->data, 2);
    ASSERT_TRUE(fn != bn);
    ASSERT_CMP(qe_size(q), 2, "node peeks must not modify the queue");
    qe_destroy_header(q);
})

TEST_CASE(test_qe_front_back_peek, {
    node* q = qe_init_header(node);
    int f = 0xDEAD, b = 0xDEAD;
    ASSERT_CMP(qe_front(q, f), NULL);
    ASSERT_CMP(qe_back(q, b), NULL);
    ASSERT_CMP(f, 0xDEAD, "out must stay untouched when empty");
    ASSERT_CMP(b, 0xDEAD);

    qe_push(q, 10);
    qe_push(q, 20);
    ASSERT_TRUE(qe_front(q, f) != NULL);
    ASSERT_TRUE(qe_back(q, b) != NULL);
    ASSERT_CMP(f, 10);
    ASSERT_CMP(b, 20);
    ASSERT_CMP(qe_size(q), 2, "peek must not modify the queue");
    qe_destroy_header(q);
})

TEST_CASE(test_qe_size, {
    node* q = qe_init_header(node);
    int v   = 0;
    ASSERT_CMP(qe_size(q), 0);
    qe_push(q, v);
    qe_push(q, v);
    ASSERT_CMP(qe_size(q), 2);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 1);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 0);
    qe_pop(q, v);
    ASSERT_CMP(qe_size(q), 0, "failed pop must not underflow size");
    qe_destroy_header(q);
})

TEST_CASE(test_qe_clear, {
    node* q = qe_init_header(node);
    for (int i = 0; i < 3; i++) qe_push(q, i);
    qe_clear(q);
    queue* hdr = qe_get_header(q);
    ASSERT_CMP(hdr->front, NULL);
    ASSERT_CMP(hdr->back, NULL);
    ASSERT_CMP(hdr->size, 0);
    qe_clear(q); /* clear on empty is a safe no-op */
    ASSERT_TRUE(qe_is_empty(q));

    /* queue is reusable after clear */
    ASSERT_TRUE(qe_push(q, 77));
    int out = 0;
    ASSERT_TRUE(qe_pop(q, out));
    ASSERT_CMP(out, 77);

    qe_destroy_header(q);
    qe_destroy_header(q); /* destroy on NULL is a safe no-op */
})

TEST_CASE(test_qe_for_each, {
    node* q    = qe_init_header(node);
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) qe_push(q, vals[i]);

    int i = 0;
    qe_for_each(q, it) { ASSERT_CMP(it->data, vals[i++]); }
    ASSERT_CMP(i, 3);

    /* iteration starts from the new front after a pop */
    int out = 0;
    qe_pop(q, out);
    ASSERT_CMP(out, vals[0]);
    i = 1;
    qe_for_each(q, it) { ASSERT_CMP(it->data, vals[i++]); }
    ASSERT_CMP(i, 3);

    qe_clear(q);
    int count = 0;
    qe_for_each(q, it) {
        (void)it;
        count++;
    }
    ASSERT_CMP(count, 0, "for_each on empty queue must not iterate");
    qe_destroy_header(q);
})

int main(void) {
    START_SUITE;
    RUN(test_qe_init);
    RUN(test_qe_fifo_order);
    RUN(test_qe_pop_empty_and_last);
    RUN(test_qe_interleaved);
    RUN(test_qe_front_back_node);
    RUN(test_qe_front_back_peek);
    RUN(test_qe_size);
    RUN(test_qe_clear);
    RUN(test_qe_for_each);
    END_SUITE;
}
