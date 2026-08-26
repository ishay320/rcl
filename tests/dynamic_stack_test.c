#include "stack/dynamic_stack.h"

#include <limits.h>

#include "test_suite.h"

typedef struct {
    size_t size;
    size_t capacity;
    int* data;
} ds_i;

void test_ds_push_and_top(void) {
    ds_i s = {0};
    ASSERT_TRUE(ds_push(s, 10));
    ASSERT_TRUE(ds_push(s, 20));
    ASSERT_TRUE(ds_push(s, 30));
    ASSERT_CMP(ds_top(s), 30);
    ASSERT_CMP(ds_size(s), (size_t)3);
    ds_destroy(&s);
}

void test_ds_pop_order(void) {
    ds_i s     = {0};
    int vals[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) ds_push(s, vals[i]);

    for (int i = 4; i >= 0; i--) {
        ASSERT_CMP(ds_top(s), vals[i]);
        int v = ds_pop(s);
        ASSERT_CMP(v, vals[i]);
    }
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
}

void test_ds_is_empty(void) {
    ds_i s = {0};
    ASSERT_TRUE(ds_is_empty(s));
    ds_push(s, 42);
    ASSERT_FALSE(ds_is_empty(s));
    (void)ds_pop(s);
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
}

void test_ds_single_element(void) {
    ds_i s = {0};
    ASSERT_TRUE(ds_push(s, 99));
    ASSERT_CMP(ds_size(s), (size_t)1);
    ASSERT_CMP(ds_top(s), 99);
    int v = ds_pop(s);
    ASSERT_CMP(v, 99);
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
}

void test_ds_boundary_values(void) {
    ds_i s     = {0};
    int vals[] = {0, -1, INT_MIN, INT_MAX};
    for (int i = 0; i < 4; i++) ds_push(s, vals[i]);

    for (int i = 3; i >= 0; i--) ASSERT_CMP(ds_pop(s), vals[i]);
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
}

void test_ds_push_many(void) {
    ds_i s = {0};
    for (int i = 0; i < 100; i++) ASSERT_TRUE(ds_push(s, i));
    ASSERT_CMP(ds_size(s), (size_t)100);
    ASSERT_CMP(ds_top(s), 99);
    for (int i = 99; i >= 0; i--) ASSERT_CMP(ds_pop(s), i);
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
}

void test_ds_size(void) {
    ds_i s = {0};
    ASSERT_CMP(ds_size(s), (size_t)0);
    ds_push(s, 1);
    ASSERT_CMP(ds_size(s), (size_t)1);
    ds_push(s, 2);
    ds_push(s, 3);
    ASSERT_CMP(ds_size(s), (size_t)3);
    (void)ds_pop(s);
    ASSERT_CMP(ds_size(s), (size_t)2);
    ds_destroy(&s);
}

void test_ds_reuse_after_empty(void) {
    ds_i s = {0};
    ds_push(s, 1);
    ds_push(s, 2);
    (void)ds_pop(s);
    (void)ds_pop(s);

    ds_push(s, 10);
    ds_push(s, 20);
    ASSERT_CMP(ds_size(s), (size_t)2);
    ASSERT_CMP(ds_top(s), 20);
    ds_destroy(&s);
}

int main(void) {
    START_SUITE;
    RUN(test_ds_push_and_top);
    RUN(test_ds_pop_order);
    RUN(test_ds_is_empty);
    RUN(test_ds_single_element);
    RUN(test_ds_boundary_values);
    RUN(test_ds_push_many);
    RUN(test_ds_size);
    RUN(test_ds_reuse_after_empty);
    END_SUITE;
    return 0;
}
