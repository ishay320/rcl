#include "queue/queue_ll_direct.h"

#include <stddef.h>

#include "test_suite.h"

/* QD_QUEUE_DEFINE declares `node` and `node_q` */
QD_QUEUE_DEFINE(node, int);

TEST_CASE(test_qd_fifo_order, {
    node_q q   = {0};
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(qd_push(&q, vals[i]));
    ASSERT_FALSE(qd_is_empty(&q));

    int out = 0;
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(qd_pop(&q, out));
        ASSERT_CMP(out, vals[i]);
    }
    ASSERT_TRUE(qd_is_empty(&q));
})

TEST_CASE(test_qd_pop_empty_and_last, {
    node_q q = {0};
    int out  = 0xDEAD;
    ASSERT_FALSE(qd_pop(&q, out));
    ASSERT_CMP(out, 0xDEAD, "out must stay untouched on empty pop");

    qd_push(&q, 99);
    ASSERT_TRUE(qd_pop(&q, out));
    ASSERT_CMP(out, 99);
    ASSERT_CMP(q.front, NULL, "popping the last node must reset front");
    ASSERT_CMP(q.back, NULL, "popping the last node must reset back");
    ASSERT_FALSE(qd_pop(&q, out));

    /* queue is reusable after being drained */
    ASSERT_TRUE(qd_push(&q, 7));
    ASSERT_TRUE(qd_pop(&q, out));
    ASSERT_CMP(out, 7);
})

TEST_CASE(test_qd_interleaved, {
    node_q q = {0};
    int out  = 0;
    qd_push(&q, 10);
    qd_push(&q, 20);
    qd_pop(&q, out);
    ASSERT_CMP(out, 10);
    qd_push(&q, 30); /* push after a partial drain keeps order */
    qd_pop(&q, out);
    ASSERT_CMP(out, 20);
    qd_pop(&q, out);
    ASSERT_CMP(out, 30);
})

TEST_CASE(test_qd_front_back_peek, {
    node_q q = {0};
    int f = 0xDEAD, b = 0xDEAD;
    ASSERT_CMP(qd_front(&q, f), NULL);
    ASSERT_CMP(qd_back(&q, b), NULL);
    ASSERT_CMP(f, 0xDEAD, "out must stay untouched when empty");
    ASSERT_CMP(b, 0xDEAD);

    qd_push(&q, 10);
    ASSERT_CMP(qd_front(&q, f), qd_back(&q, b), "single node is both front and back");
    ASSERT_CMP(f, 10);
    ASSERT_CMP(b, 10);

    qd_push(&q, 20);
    node* fn = qd_front(&q, f);
    node* bn = qd_back(&q, b);
    ASSERT_TRUE(fn != NULL);
    ASSERT_TRUE(fn != bn);
    ASSERT_CMP(f, 10);
    ASSERT_CMP(b, 20);
    ASSERT_CMP(qd_size(&q), 2, "peek must not modify the queue");
    qd_clear(&q);
})

TEST_CASE(test_qd_size, {
    node_q q = {0};
    int v    = 0;
    ASSERT_CMP(qd_size(&q), 0);
    qd_push(&q, v);
    qd_push(&q, v);
    ASSERT_CMP(qd_size(&q), 2);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 1);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 0);
    qd_pop(&q, v);
    ASSERT_CMP(qd_size(&q), 0, "failed pop must not underflow size");
})

TEST_CASE(test_qd_clear, {
    node_q q = {0};
    for (int i = 0; i < 3; i++) qd_push(&q, i);
    qd_clear(&q);
    ASSERT_CMP(q.front, NULL);
    ASSERT_CMP(q.back, NULL);
    ASSERT_CMP(q.size, 0);
    qd_clear(&q); /* clear on empty is a safe no-op */
    ASSERT_TRUE(qd_is_empty(&q));

    /* queue is reusable after clear */
    ASSERT_TRUE(qd_push(&q, 77));
    int out = 0;
    ASSERT_TRUE(qd_pop(&q, out));
    ASSERT_CMP(out, 77);
})

typedef struct {
    int id;
    float score;
} item;

/* QD_QUEUE_DEFINE with a non-scalar data type */
QD_QUEUE_DEFINE(item_node, item);

TEST_CASE(test_qd_struct_data, {
    item_node_q q = {0};
    for (int i = 0; i < 3; i++) {
        item it = {.id = i, .score = (float)i * 0.5f};
        ASSERT_TRUE(qd_push(&q, it));
    }
    for (int i = 0; i < 3; i++) {
        item out = {0};
        ASSERT_TRUE(qd_pop(&q, out));
        ASSERT_CMP(out.id, i);
        ASSERT_CMP(out.score, (float)i * 0.5f);
    }
    ASSERT_TRUE(qd_is_empty(&q));
})
