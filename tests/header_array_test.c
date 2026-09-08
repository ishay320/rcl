#define HEADER_ARRAY_IMPLEMENTATION
#include "arrays/header_array.h"

#include "test_suite.h"

typedef struct {
    int id;
    float value;
    char tag[8];
} test_item_t;

TEST_CASE(test_array_init_sets_metadata, {
    int* arr = array_init_sized(int, 10);
    ASSERT_TRUE(arr != NULL);
    ASSERT_CMP(array_cap(arr), (size_t)10);
    ASSERT_CMP(array_len(arr), (size_t)0);
    ASSERT_CMP(array_stride(arr), sizeof(int));
    array_free(arr);

    test_item_t* structs = array_init(test_item_t);
    ASSERT_TRUE(structs != NULL);
    ASSERT_CMP(array_cap(structs), (size_t)1, "array_init starts with capacity 1");
    ASSERT_CMP(array_stride(structs), sizeof(test_item_t));
    array_free(structs);
})

TEST_CASE(test_array_push_lvalue_and_rvalue, {
    int* arr = array_init(int);
    int lval = 42;
    ASSERT_TRUE(array_push(arr, lval));
    ASSERT_TRUE(array_push_rval(arr, 10 + 3), "push_rval must accept temporaries");
    ASSERT_CMP(array_len(arr), (size_t)2);
    ASSERT_CMP(arr[0], 42);
    ASSERT_CMP(arr[1], 13);
    array_free(arr);
})

TEST_CASE(test_array_capacity_doubles_when_full, {
    int* arr = array_init(int);
    for (int i = 0; i < 100; i++) ASSERT_TRUE(array_push_rval(arr, i));

    ASSERT_CMP(array_len(arr), (size_t)100);
    ASSERT_CMP(array_cap(arr), (size_t)128, "1 -> 2 -> 4 -> ... -> 128");
    for (int i = 0; i < 100; i++) ASSERT_CMP(arr[i], i);
    array_free(arr);
})

TEST_CASE(test_array_reserve_only_grows, {
    int* arr = array_init(int);
    ASSERT_TRUE(array_reserve(arr, 10));
    ASSERT_CMP(array_cap(arr), (size_t)10);
    ASSERT_CMP(array_len(arr), (size_t)0, "reserve must not change length");

    ASSERT_TRUE(array_reserve(arr, 5));
    ASSERT_CMP(array_cap(arr), (size_t)10, "smaller reserve is a no-op");
    ASSERT_TRUE(array_reserve(arr, 30));
    ASSERT_CMP(array_cap(arr), (size_t)30);
    array_free(arr);
})

TEST_CASE(test_array_resize_shrink_clamps_len, {
    int* arr = array_init(int);
    for (int i = 0; i < 5; i++) array_push_rval(arr, i * 10);

    ASSERT_TRUE(array_resize(arr, 30));
    ASSERT_CMP(array_cap(arr), (size_t)30);
    ASSERT_CMP(array_len(arr), (size_t)5, "growing must not change length");

    ASSERT_TRUE(array_resize(arr, 3));
    ASSERT_CMP(array_cap(arr), (size_t)3);
    ASSERT_CMP(array_len(arr), (size_t)3, "shrinking clamps length to the new capacity");
    for (int i = 0; i < 3; i++) ASSERT_CMP(arr[i], i * 10);

    array_free(arr);
})

TEST_CASE(test_array_push_grows_from_zero_capacity, {
    int* arr = array_init_sized(int, 0);
    ASSERT_TRUE(arr != NULL);
    ASSERT_CMP(array_cap(arr), (size_t)0);

    ASSERT_TRUE(array_push_rval(arr, 7));
    ASSERT_CMP(array_len(arr), (size_t)1);
    ASSERT_CMP(arr[0], 7);

    ASSERT_TRUE(array_resize(arr, 0), "resize to zero is allowed");
    ASSERT_CMP(array_len(arr), (size_t)0);
    ASSERT_TRUE(array_push_rval(arr, 9), "push after resize to zero must reallocate");
    ASSERT_CMP(arr[0], 9);

    array_free(arr);
})

TEST_CASE(test_array_remove_index_preserves_order, {
    int* arr = array_init(int);
    for (int i = 0; i < 5; i++) array_push_rval(arr, i * 10); /* {0,10,20,30,40} */

    array_remove_index(arr, 1); /* middle -> {0,20,30,40} */
    ASSERT_CMP(array_len(arr), (size_t)4);
    ASSERT_CMP(arr[0], 0);
    ASSERT_CMP(arr[1], 20);
    ASSERT_CMP(arr[3], 40);

    array_remove_index(arr, 3); /* last -> {0,20,30} */
    ASSERT_CMP(array_len(arr), (size_t)3);
    ASSERT_CMP(arr[2], 30);

    array_remove_index(arr, 0); /* first -> {20,30} */
    ASSERT_CMP(array_len(arr), (size_t)2);
    ASSERT_CMP(arr[0], 20);

    array_remove_index(arr, 0);
    array_remove_index(arr, 0);
    ASSERT_CMP(array_len(arr), (size_t)0, "array can be emptied via remove");

    array_push_rval(arr, 99); /* still usable after removing everything */
    ASSERT_CMP(arr[0], 99);

    array_free(arr);
})

TEST_CASE(test_array_struct_elements, {
    test_item_t* arr = array_init(test_item_t);
    test_item_t a    = {.id = 1, .value = 1.5f, .tag = "one"};
    test_item_t b    = {.id = 2, .value = 2.5f, .tag = "two"};
    test_item_t c    = {.id = 3, .value = 3.5f, .tag = "three"};
    ASSERT_TRUE(array_push(arr, a));
    ASSERT_TRUE(array_push(arr, b));
    ASSERT_TRUE(array_push(arr, c));

    ASSERT_CMP(arr[1].id, 2);
    ASSERT_CMP(arr[1].value, 2.5f);
    ASSERT_CMP(arr[2].tag, "three");

    array_remove_index(arr, 1);
    ASSERT_CMP(array_len(arr), (size_t)2);
    ASSERT_CMP(arr[0].id, 1);
    ASSERT_CMP(arr[1].id, 3);
    ASSERT_CMP(arr[1].tag, "three");

    array_free(arr);
})
