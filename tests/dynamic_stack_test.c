#include "stack/dynamic_stack.h"

#include <limits.h>

#include "test_suite.h"

typedef struct {
    size_t size;
    size_t capacity;
    int* data;
} ds_i;
typedef struct {
    int id;
    float weight;
} item_t;
typedef struct {
    size_t size;
    size_t capacity;
    item_t* data;
} ds_item;

TEST_CASE(test_ds_lifo_push_pop_order, {
    ds_i s = {0};
    ASSERT_TRUE(ds_is_empty(s), "a zero-initialized stack is empty");
    ASSERT_CMP(ds_size(s), (size_t)0);

    int vals[] = {INT_MIN, -1, 0, 42, INT_MAX};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(ds_push(s, vals[i]));
    ASSERT_CMP(ds_size(s), (size_t)5);
    ASSERT_FALSE(ds_is_empty(s));

    for (int i = 4; i >= 0; i--) {
        ASSERT_CMP(ds_top(s), vals[i], "top must peek without removing");
        ASSERT_CMP(ds_pop(s), vals[i]);
        ASSERT_CMP(ds_size(s), (size_t)i);
    }
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
})

TEST_CASE(test_ds_grows_past_initial_capacity, {
    ds_i s = {0};
    for (int i = 0; i < 100; i++) ASSERT_TRUE(ds_push(s, i));
    ASSERT_CMP(ds_size(s), (size_t)100);
    ASSERT_CMP(ds_top(s), 99);

    for (int i = 99; i >= 0; i--) ASSERT_CMP(ds_pop(s), i);
    ASSERT_TRUE(ds_is_empty(s));
    ds_destroy(&s);
})

TEST_CASE(test_ds_interleaved_push_pop_and_reuse, {
    ds_i s = {0};
    ds_push(s, 1);
    ds_push(s, 2);
    ASSERT_CMP(ds_pop(s), 2);

    ds_push(s, 3); /* reuses the popped slot */
    ASSERT_CMP(ds_top(s), 3);
    ASSERT_CMP(ds_size(s), (size_t)2);
    ASSERT_CMP(ds_pop(s), 3);
    ASSERT_CMP(ds_pop(s), 1);
    ASSERT_TRUE(ds_is_empty(s));

    ds_push(s, 10); /* a drained stack is reusable */
    ds_push(s, 20);
    ASSERT_CMP(ds_size(s), (size_t)2);
    ASSERT_CMP(ds_top(s), 20);
    ds_destroy(&s);
})

TEST_CASE(test_ds_struct_elements, {
    ds_item s = {0};
    item_t a  = {.id = 1, .weight = 1.5f};
    item_t b  = {.id = 2, .weight = 2.5f};
    ASSERT_TRUE(ds_push(s, a));
    ASSERT_TRUE(ds_push(s, b));

    ASSERT_CMP(ds_top(s).id, 2);
    item_t popped = ds_pop(s);
    ASSERT_CMP(popped.id, 2);
    ASSERT_CMP(popped.weight, 2.5f);
    ASSERT_CMP(ds_top(s).id, 1, "pop must expose the element below");
    ds_destroy(&s);
})
