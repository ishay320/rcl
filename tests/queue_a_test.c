#include "queue/queue_a.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "ansi.h"
#include "test_suite.h"

typedef struct queue {
    size_t start;
    size_t end;
    size_t size;
    size_t cap;
    int* data;
} queue;

void test_qa_fifo_order(void) {
    queue q    = qa_init(queue, 5);
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) assert(qa_push(q, vals[i]));

    int out;
    for (int i = 0; i < 5; i++) {
        assert(qa_pop(q, out));
        assert(out == vals[i]);
    }
    assert(qa_is_empty(q));
    qa_destroy(q);
}

void test_qa_is_empty(void) {
    queue q = qa_init(queue, 4);
    assert(qa_is_empty(q));
    qa_push(q, 1);
    assert(!qa_is_empty(q));
    int out = 0;
    qa_pop(q, out);
    (void)out;
    assert(qa_is_empty(q));
    qa_destroy(q);
}

void test_qa_is_full(void) {
    queue q = qa_init(queue, 3);
    assert(qa_push(q, 1));
    assert(qa_push(q, 2));
    assert(qa_push(q, 3));
    assert(qa_is_full(q));
    assert(!qa_push(q, 4));
    assert(qa_size(q) == 3);
    qa_destroy(q);
}

void test_qa_front_back(void) {
    queue q = qa_init(queue, 4);
    qa_push(q, 11);
    qa_push(q, 22);
    qa_push(q, 33);

    int f, b;
    assert(qa_front(q, f));
    assert(f == 11);
    assert(qa_back(q, b));
    assert(b == 33);
    assert(qa_size(q) == 3); /* peek must not modify size */
    qa_destroy(q);
}

void test_qa_circular_wrap(void) {
    queue q = qa_init(queue, 4);
    qa_push(q, 1);
    qa_push(q, 2);
    qa_push(q, 3);
    qa_push(q, 4);

    int out = 0;
    qa_pop(q, out);
    assert(out == 1);
    qa_pop(q, out);
    assert(out == 2);

    assert(qa_push(q, 5));
    assert(qa_push(q, 6));
    assert(qa_is_full(q));

    qa_pop(q, out);
    assert(out == 3);
    qa_pop(q, out);
    assert(out == 4);
    qa_pop(q, out);
    assert(out == 5);
    qa_pop(q, out);
    assert(out == 6);
    assert(qa_is_empty(q));
    qa_destroy(q);
}

void test_qa_boundary_values(void) {
    queue q    = qa_init(queue, 4);
    int vals[] = {0, -1, INT_MIN, INT_MAX};
    for (int i = 0; i < 4; i++) assert(qa_push(q, vals[i]));

    int out;
    for (int i = 0; i < 4; i++) {
        assert(qa_pop(q, out));
        assert(out == vals[i]);
    }
    qa_destroy(q);
}

void test_qa_clear(void) {
    queue q = qa_init(queue, 3);
    qa_push(q, 1);
    qa_push(q, 2);
    qa_clear(q);
    assert(qa_is_empty(q));
    assert(qa_size(q) == 0);

    assert(qa_push(q, 99));
    int out;
    assert(qa_pop(q, out));
    assert(out == 99);
    qa_destroy(q);
}

void test_qa_destroy(void) {
    queue q = qa_init(queue, 4);
    qa_push(q, 7);
    qa_destroy(q);
    assert(q.data == NULL);
    assert(q.size == 0);
    assert(q.cap == 0);
}

void test_qa_pop_empty(void) {
    queue q = qa_init(queue, 4);
    int out = 0;
    assert(!qa_pop(q, out));
    assert(out == 0); /* out unchanged */
    qa_destroy(q);
}

void test_qa_front_back_empty(void) {
    queue q = qa_init(queue, 4);
    int f = 0, b = 0;
    assert(!qa_front(q, f));
    assert(!qa_back(q, b));
    assert(f == 0); /* out unchanged */
    assert(b == 0);
    qa_destroy(q);
}

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

void test_qa_big_struct(void) {
    queue_big q = qa_init(queue_big, 8);

    for (int i = 0; i < 8; i++) {
        big_item item = {.id = i, .score = (float)i * 1.5f};
        assert(qa_push(q, item));
    }
    assert(qa_is_full(q));

    for (int i = 0; i < 8; i++) {
        big_item out;
        assert(qa_pop(q, out));
        assert(out.id == i);
        assert(out.score == (float)i * 1.5f);
    }
    assert(qa_is_empty(q));
    qa_destroy(q);
}

int main(void) {
    START_SUITE;
    RUN(test_qa_fifo_order);
    RUN(test_qa_is_empty);
    RUN(test_qa_is_full);
    RUN(test_qa_front_back);
    RUN(test_qa_circular_wrap);
    RUN(test_qa_boundary_values);
    RUN(test_qa_clear);
    RUN(test_qa_destroy);
    RUN(test_qa_pop_empty);
    RUN(test_qa_front_back_empty);
    RUN(test_qa_big_struct);
    END_SUITE;
    return 0;
}