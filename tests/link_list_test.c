#include "link_list/link_list.h"

#include <assert.h> /* link_list.h uses assert internally */
#include <stdlib.h>

#include "test_suite.h"

typedef struct node_i_s {
    struct node_i_s* next;
    int data;
} node_i_s;

static int find_data_1(node_i_s* n) { return n->data == 1; }
static int find_data_2(node_i_s* n) { return n->data == 2; }
static int find_data_99(node_i_s* n) { return n->data == 99; }

void test_ll_insert_next(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    ASSERT_CMP(a.next, &b);
    ASSERT_CMP(b.next, &c);
    ASSERT_CMP(a.next->next->data, 3);
}

void test_ll_is_empty(void) {
    node_i_s a = {.data = 1};
    ASSERT_TRUE(ll_is_empty(a));
    node_i_s b = {.data = 2};
    ll_insert_next(&a, &b);
    ASSERT_FALSE(ll_is_empty(a));
}

void test_ll_remove_next_middle(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    node_i_s* n = ll_remove_next(&a);
    ASSERT_CMP(a.next, &c);
    ASSERT_CMP(a.next->data, 3);
    ASSERT_CMP(&b, n);
}

void test_ll_remove_next_last(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    ll_insert_next(&a, &b);
    ll_remove_next(&a);
    ASSERT_CMP(a.next, NULL);
}

void test_ll_count(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ASSERT_CMP(ll_count(a), (size_t)1);
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    ASSERT_CMP(ll_count(a), (size_t)3);
}

void test_ll_find(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    node_i_s* head_found = ll_find(a, find_data_1);
    ASSERT_CMP(head_found, &a);
    node_i_s* found = ll_find(a, find_data_2);
    ASSERT_CMP(found, &b);
    node_i_s* not_found = ll_find(a, find_data_99);
    ASSERT_CMP(not_found, NULL);
}

void test_ll_insert_tail_single(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    ll_insert_tail(&a, &b);
    ASSERT_CMP(a.next, &b);
    ASSERT_CMP(b.next, NULL);
}

void test_ll_insert_tail_chain(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_tail(&a, &c);
    ASSERT_CMP(a.next, &b);
    ASSERT_CMP(b.next, &c);
    ASSERT_CMP(c.next, NULL);
}

void test_ll_insert_tail_deep(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    node_i_s d = {.data = 4};
    ll_insert_next(&a, &b);
    ll_insert_tail(&b, &c);
    ll_insert_tail(&a, &d);
    ASSERT_CMP(c.next, &d);
    ASSERT_CMP(d.next, NULL);
}

void test_ll_for_each(void) {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    int sum = 0;
    ll_for_each(a, i) { sum += i->data; }
    ASSERT_CMP(sum, 6);
}

void test_ll_free(void) {
    node_i_s* base = calloc(1, sizeof(node_i_s));
    base->data     = 1;
    for (int i = 0; i < 10; i++) {
        node_i_s* n = calloc(1, sizeof(node_i_s));
        n->data     = i;
        ll_insert_tail(base, n);
    }
    ASSERT_CMP(ll_count(*base), (size_t)11);

    for (typeof(base) cur = base, next; cur != NULL; cur = next) {
        next = cur->next;
        free(cur);
    }
}

int main(void) {
    START_SUITE;
    RUN(test_ll_insert_next);
    RUN(test_ll_is_empty);
    RUN(test_ll_remove_next_middle);
    RUN(test_ll_remove_next_last);
    RUN(test_ll_count);
    RUN(test_ll_find);
    RUN(test_ll_insert_tail_single);
    RUN(test_ll_insert_tail_chain);
    RUN(test_ll_insert_tail_deep);
    RUN(test_ll_for_each);
    RUN(test_ll_free);
    END_SUITE;
    return 0;
}
