#include "arrays/dynamic_array.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "test_suite.h"

typedef struct {
    size_t size;
    size_t capacity;
    int* data;
} da_i;
typedef struct {
    size_t size;
    size_t capacity;
    void** data;
} da_p;
typedef struct {
    size_t size;
    size_t capacity;
    char** data;
} da_s;

TEST_CASE(test_da_i_append, {
    da_i arr   = {0};
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(da_append(arr, vals[i]));

    ASSERT_CMP(arr.size, (size_t)5);
    ASSERT_TRUE(arr.capacity >= 5);
    for (int i = 0; i < 5; i++) ASSERT_CMP(arr.data[i], vals[i]);

    da_destroy(&arr);
})

TEST_CASE(test_da_s_append_counting, {
    da_s arr = {0};
    ASSERT_TRUE(da_append(arr, "test one size"));

    ASSERT_CMP(arr.size, (size_t)1);
    ASSERT_TRUE(arr.capacity >= 1);
    ASSERT_CMP(arr.data[0], "test one size");

    da_destroy(&arr);
})

TEST_CASE(test_da_i_double_destroy, {
    da_i arr = {0};
    da_append(arr, 1);
    da_destroy(&arr);
    ASSERT_CMP(arr.data, NULL);
    ASSERT_CMP(arr.size, (size_t)0);
    ASSERT_CMP(arr.capacity, (size_t)0);
    da_destroy(&arr);
})

TEST_CASE(test_da_p_empty, {
    da_p empty = {0};
    ASSERT_CMP(empty.size, (size_t)0);
    ASSERT_CMP(empty.capacity, (size_t)0);
    da_destroy(&empty);
})

TEST_CASE(test_da_p_append, {
    da_p arr = {0};
    int a = 1, b = 2, c = 3;
    void* vals[] = {&a, &b, &c};
    for (int i = 0; i < 3; i++) ASSERT_TRUE(da_append(arr, vals[i]));

    ASSERT_CMP(arr.size, (size_t)3);
    for (int i = 0; i < 3; i++) ASSERT_CMP(arr.data[i], vals[i]);

    da_destroy(&arr);
    ASSERT_CMP(arr.data, NULL);
})

TEST_CASE(test_da_i_single_element, {
    da_i arr = {0};
    ASSERT_TRUE(da_append(arr, 42));
    ASSERT_CMP(arr.size, (size_t)1);
    ASSERT_CMP(arr.capacity, (size_t)DA_INITIAL_CAPACITY);
    ASSERT_CMP(arr.data[0], 42);
    da_destroy(&arr);
})

TEST_CASE(test_da_i_reuse_after_destroy, {
    da_i arr = {0};
    for (int i = 0; i < 5; i++) da_append(arr, i * 10);
    da_destroy(&arr);

    for (int i = 0; i < 3; i++) da_append(arr, i + 100);
    ASSERT_CMP(arr.size, (size_t)3);
    ASSERT_CMP(arr.data[0], 100);
    ASSERT_CMP(arr.data[1], 101);
    ASSERT_CMP(arr.data[2], 102);

    da_destroy(&arr);
})

TEST_CASE(test_da_i_capacity_growth, {
    da_i arr = {0};
    for (int i = 0; i < 50; i++) da_append(arr, i);

    ASSERT_CMP(arr.size, (size_t)50);
    ASSERT_TRUE(arr.capacity >= 50);
    ASSERT_CMP((arr.capacity & (arr.capacity - 1)), (size_t)0);
    ASSERT_CMP(arr.data[0], 0);
    ASSERT_CMP(arr.data[49], 49);

    da_destroy(&arr);
})

TEST_CASE(test_da_i_boundary_values, {
    da_i arr   = {0};
    int vals[] = {0, -1, -100, INT_MIN, INT_MAX};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(da_append(arr, vals[i]));

    ASSERT_CMP(arr.size, (size_t)5);
    for (int i = 0; i < 5; i++) ASSERT_CMP(arr.data[i], vals[i]);

    da_destroy(&arr);
})

TEST_CASE(test_da_i_reserve, {
    da_i arr = {0};
    ASSERT_TRUE(da_reserve(arr, 100));
    ASSERT_CMP(arr.capacity, (size_t)100);
    ASSERT_CMP(arr.size, (size_t)0);

    ASSERT_TRUE(da_reserve(arr, 10));
    ASSERT_CMP(arr.capacity, (size_t)100);

    da_append(arr, 42);
    ASSERT_CMP(arr.size, (size_t)1);
    ASSERT_CMP(arr.data[0], 42);

    da_destroy(&arr);
})

int main(void) {
    START_SUITE;
    RUN(test_da_i_append);
    RUN(test_da_s_append_counting);
    RUN(test_da_i_double_destroy);
    RUN(test_da_p_empty);
    RUN(test_da_p_append);
    RUN(test_da_i_single_element);
    RUN(test_da_i_reuse_after_destroy);
    RUN(test_da_i_capacity_growth);
    RUN(test_da_i_boundary_values);
    RUN(test_da_i_reserve);
    END_SUITE;
    return 0;
}
