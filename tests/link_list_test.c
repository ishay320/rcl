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

TEST_CASE(test_ll_insert_next, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ASSERT_TRUE(ll_is_empty(a));
    ll_insert_next(&a, &b);
    ASSERT_FALSE(ll_is_empty(a));
    ll_insert_next(&b, &c);
    ASSERT_CMP(a.next, &b);
    ASSERT_CMP(b.next, &c);
})

TEST_CASE(test_ll_remove_next, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);

    node_i_s* removed = ll_remove_next(&a); /* middle node */
    ASSERT_CMP(removed, &b);
    ASSERT_CMP(a.next, &c, "removing the middle node must relink around it");

    removed = ll_remove_next(&a); /* last node */
    ASSERT_CMP(removed, &c);
    ASSERT_CMP(a.next, NULL);
    ASSERT_TRUE(ll_is_empty(a));
})

TEST_CASE(test_ll_count, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ASSERT_CMP(ll_count(a), (size_t)1, "a lone head counts as one node");
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    ASSERT_CMP(ll_count(a), (size_t)3);
})

TEST_CASE(test_ll_find, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    node_i_s* head_found = ll_find(a, find_data_1);
    ASSERT_CMP(head_found, &a, "search must include the head itself");
    node_i_s* found = ll_find(a, find_data_2);
    ASSERT_CMP(found, &b);
    node_i_s* not_found = ll_find(a, find_data_99);
    ASSERT_CMP(not_found, NULL);
})

TEST_CASE(test_ll_insert_tail, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    node_i_s d = {.data = 4};
    ll_insert_tail(&a, &b);
    ASSERT_CMP(a.next, &b);
    ASSERT_CMP(b.next, NULL);
    ll_insert_tail(&a, &c); /* must walk from head to the real tail */
    ASSERT_CMP(b.next, &c);
    ll_insert_tail(&a, &d);
    ASSERT_CMP(c.next, &d);
    ASSERT_CMP(d.next, NULL);
})

TEST_CASE(test_ll_for_each, {
    node_i_s a = {.data = 1};
    node_i_s b = {.data = 2};
    node_i_s c = {.data = 3};
    ll_insert_next(&a, &b);
    ll_insert_next(&b, &c);
    int sum = 0;
    ll_for_each(a, i) { sum += i->data; }
    ASSERT_CMP(sum, 6, "iteration must start from the head node itself");
})

TEST_CASE(test_ll_for_each_free, {
    node_i_s* head = calloc(1, sizeof(node_i_s));
    head->data     = 0;
    for (int i = 1; i <= 10; i++) {
        node_i_s* n = calloc(1, sizeof(node_i_s));
        n->data     = i;
        ll_insert_tail(head, n);
    }
    ASSERT_CMP(ll_count(*head), (size_t)11);
    /* ASAN verifies every node is freed exactly once, with no use after free */
    ll_for_each_free(*head, free);
})

int main(void) {
    START_SUITE;
    RUN(test_ll_insert_next);
    RUN(test_ll_remove_next);
    RUN(test_ll_count);
    RUN(test_ll_find);
    RUN(test_ll_insert_tail);
    RUN(test_ll_for_each);
    RUN(test_ll_for_each_free);
    END_SUITE;
}
