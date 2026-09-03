#include "queue/queue_a.h"

#include <stddef.h>

#include "test_suite.h"

typedef struct queue {
    size_t start;
    size_t end;
    size_t size;
    size_t cap;
    int* data;
} queue;

TEST_CASE(test_qa_fifo_order, {
    queue q    = qa_init(queue, 5);
    int vals[] = {10, 20, 30, 40, 50};
    ASSERT_TRUE(qa_is_empty(q));
    for (int i = 0; i < 5; i++) ASSERT_TRUE(qa_push(q, vals[i]));
    ASSERT_FALSE(qa_is_empty(q));
    ASSERT_CMP(qa_size(q), (size_t)5);

    int out;
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(qa_pop(q, out));
        ASSERT_CMP(out, vals[i]);
    }
    ASSERT_TRUE(qa_is_empty(q));
    qa_destroy(q);
})

TEST_CASE(test_qa_empty_ops_fail, {
    queue q = qa_init(queue, 4);
    int out = 0xDEAD, f = 0xDEAD, b = 0xDEAD;
    ASSERT_FALSE(qa_pop(q, out));
    ASSERT_FALSE(qa_front(q, f));
    ASSERT_FALSE(qa_back(q, b));
    ASSERT_CMP(out, 0xDEAD, "out must stay untouched on empty pop");
    ASSERT_CMP(f, 0xDEAD);
    ASSERT_CMP(b, 0xDEAD);
    qa_destroy(q);
})

TEST_CASE(test_qa_full_rejects_push, {
    queue q = qa_init(queue, 3);
    for (int i = 1; i <= 3; i++) ASSERT_TRUE(qa_push(q, i));
    ASSERT_TRUE(qa_is_full(q));
    ASSERT_FALSE(qa_push(q, 4));
    ASSERT_CMP(qa_size(q), (size_t)3, "failed push must not change size");

    int out;
    ASSERT_TRUE(qa_pop(q, out));
    ASSERT_CMP(out, 1, "rejected push must not overwrite the front");
    qa_destroy(q);
})

TEST_CASE(test_qa_front_back_peek, {
    queue q = qa_init(queue, 4);
    qa_push(q, 11);
    qa_push(q, 22);
    qa_push(q, 33);

    int f, b;
    ASSERT_TRUE(qa_front(q, f));
    ASSERT_CMP(f, 11);
    ASSERT_TRUE(qa_back(q, b));
    ASSERT_CMP(b, 33);
    ASSERT_CMP(qa_size(q), (size_t)3, "peek must not modify size");
    qa_destroy(q);
})

TEST_CASE(test_qa_wrap_around, {
    queue q = qa_init(queue, 4);
    for (int i = 1; i <= 4; i++) qa_push(q, i);

    int out = 0;
    qa_pop(q, out);
    qa_pop(q, out);
    ASSERT_CMP(out, 2);

    /* end index wraps past the buffer edge */
    ASSERT_TRUE(qa_push(q, 5));
    ASSERT_TRUE(qa_push(q, 6));
    ASSERT_TRUE(qa_is_full(q));

    int f, b;
    ASSERT_TRUE(qa_front(q, f));
    ASSERT_TRUE(qa_back(q, b));
    ASSERT_CMP(f, 3, "front while wrapped");
    ASSERT_CMP(b, 6, "back while wrapped");

    int expected[] = {3, 4, 5, 6};
    for (int i = 0; i < 4; i++) {
        ASSERT_TRUE(qa_pop(q, out));
        ASSERT_CMP(out, expected[i]);
    }
    ASSERT_TRUE(qa_is_empty(q));
    qa_destroy(q);
})

TEST_CASE(test_qa_capacity_one, {
    queue q = qa_init(queue, 1);
    int out;
    for (int i = 0; i < 3; i++) { /* every push/pop wraps the indices */
        ASSERT_TRUE(qa_push(q, i));
        ASSERT_TRUE(qa_is_full(q));
        ASSERT_FALSE(qa_push(q, 99));
        ASSERT_TRUE(qa_pop(q, out));
        ASSERT_CMP(out, i);
    }
    ASSERT_TRUE(qa_is_empty(q));
    qa_destroy(q);
})

TEST_CASE(test_qa_clear_after_wrap, {
    queue q = qa_init(queue, 3);
    int out;
    qa_push(q, 1);
    qa_push(q, 2);
    qa_push(q, 3);
    qa_pop(q, out);
    qa_push(q, 4); /* queue is now in a wrapped state */

    qa_clear(q);
    ASSERT_TRUE(qa_is_empty(q));
    ASSERT_CMP(qa_size(q), (size_t)0);

    /* full capacity is usable again and order is clean */
    for (int i = 10; i < 13; i++) ASSERT_TRUE(qa_push(q, i));
    ASSERT_TRUE(qa_is_full(q));
    for (int i = 10; i < 13; i++) {
        ASSERT_TRUE(qa_pop(q, out));
        ASSERT_CMP(out, i);
    }
    qa_destroy(q);
})

TEST_CASE(test_qa_destroy, {
    queue q = qa_init(queue, 4);
    qa_push(q, 7);
    qa_destroy(q);
    ASSERT_CMP(q.data, NULL);
    ASSERT_CMP(q.size, (size_t)0);
    ASSERT_CMP(q.cap, (size_t)0);
    qa_destroy(q); /* second destroy is a safe no-op */
    ASSERT_CMP(q.data, NULL);
})

typedef struct {
    int id;
    char payload[1024];
    float score;
} big_item;

typedef struct {
    size_t start;
    size_t end;
    size_t size;
    size_t cap;
    big_item* data;
} queue_big;

TEST_CASE(test_qa_big_struct, {
    queue_big q = qa_init(queue_big, 8);

    for (int i = 0; i < 8; i++) {
        big_item item = {.id = i, .score = (float)i * 1.5f};
        ASSERT_TRUE(qa_push(q, item));
    }
    ASSERT_TRUE(qa_is_full(q));

    for (int i = 0; i < 8; i++) {
        big_item out;
        ASSERT_TRUE(qa_pop(q, out));
        ASSERT_CMP(out.id, i);
        ASSERT_CMP(out.score, (float)i * 1.5f);
    }
    ASSERT_TRUE(qa_is_empty(q));
    qa_destroy(q);
})

int main(void) {
    START_SUITE;
    RUN(test_qa_fifo_order);
    RUN(test_qa_empty_ops_fail);
    RUN(test_qa_full_rejects_push);
    RUN(test_qa_front_back_peek);
    RUN(test_qa_wrap_around);
    RUN(test_qa_capacity_one);
    RUN(test_qa_clear_after_wrap);
    RUN(test_qa_destroy);
    RUN(test_qa_big_struct);
    END_SUITE;
}