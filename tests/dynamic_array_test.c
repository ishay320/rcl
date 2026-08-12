#include "arrays/dynamic_array.h"

#include <assert.h>
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

void test_da_i_append(void) {
    da_i arr   = {0};
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) assert(da_append(arr, vals[i]));

    assert(arr.size == 5);
    assert(arr.capacity >= 5);
    for (int i = 0; i < 5; i++) assert(arr.data[i] == vals[i]);

    da_destroy(&arr);
}

void test_da_s_append_counting(void) {
    da_s arr = {0};
    assert(da_append(arr, "test one size"));

    assert(arr.size == 1);
    assert(arr.capacity >= 1);
    assert(strcmp(arr.data[0], "test one size") == 0);

    da_destroy(&arr);
}

void test_da_i_double_destroy(void) {
    da_i arr = {0};
    da_append(arr, 1);
    da_destroy(&arr);
    assert(arr.data == NULL);
    assert(arr.size == 0);
    assert(arr.capacity == 0);
    da_destroy(&arr);
}

void test_da_p_empty(void) {
    da_p empty = {0};
    assert(empty.size == 0);
    assert(empty.capacity == 0);
    da_destroy(&empty);
}

void test_da_p_append(void) {
    da_p arr = {0};
    int a = 1, b = 2, c = 3;
    void* vals[] = {&a, &b, &c};
    for (int i = 0; i < 3; i++) assert(da_append(arr, vals[i]));

    assert(arr.size == 3);
    for (int i = 0; i < 3; i++) assert(arr.data[i] == vals[i]);

    da_destroy(&arr);
    assert(arr.data == NULL);
}

void test_da_i_single_element(void) {
    da_i arr = {0};
    assert(da_append(arr, 42));
    assert(arr.size == 1);
    assert(arr.capacity == DA_INITIAL_CAPACITY);
    assert(arr.data[0] == 42);
    da_destroy(&arr);
}

void test_da_i_reuse_after_destroy(void) {
    da_i arr = {0};
    for (int i = 0; i < 5; i++) da_append(arr, i * 10);
    da_destroy(&arr);

    for (int i = 0; i < 3; i++) da_append(arr, i + 100);
    assert(arr.size == 3);
    assert(arr.data[0] == 100);
    assert(arr.data[1] == 101);
    assert(arr.data[2] == 102);

    da_destroy(&arr);
}

void test_da_i_capacity_growth(void) {
    da_i arr = {0};
    for (int i = 0; i < 50; i++) da_append(arr, i);

    assert(arr.size == 50);
    assert(arr.capacity >= 50);
    assert((arr.capacity & (arr.capacity - 1)) == 0);
    assert(arr.data[0] == 0);
    assert(arr.data[49] == 49);

    da_destroy(&arr);
}

void test_da_i_boundary_values(void) {
    da_i arr   = {0};
    int vals[] = {0, -1, -100, INT_MIN, INT_MAX};
    for (int i = 0; i < 5; i++) assert(da_append(arr, vals[i]));

    assert(arr.size == 5);
    for (int i = 0; i < 5; i++) assert(arr.data[i] == vals[i]);

    da_destroy(&arr);
}

void test_da_i_reserve(void) {
    da_i arr = {0};
    assert(da_reserve(arr, 100));
    assert(arr.capacity == 100);
    assert(arr.size == 0);

    assert(da_reserve(arr, 10));
    assert(arr.capacity == 100);

    da_append(arr, 42);
    assert(arr.size == 1);
    assert(arr.data[0] == 42);

    da_destroy(&arr);
}

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
