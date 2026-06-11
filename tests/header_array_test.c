#define HEADER_ARRAY_IMPLEMENTATION
#include "arrays/header_array.h"

#include <assert.h>

#include "test_suite.h"

void test_array_initialization(void) {
    int* arr = array_init(int);
    assert(arr != NULL);
    array_free(arr);
}

void test_array_init_sized(void) {
    int* arr = array_init_sized(int, 10);
    assert(arr != NULL);
    struct array* header = array_get_header(arr);
    assert(header->cap == 10);
    array_free(arr);
}

void test_array_push_rval(void) {
    int* arr = array_init(int);
    array_push_rval(arr, 42);
    struct array* header = array_get_header(arr);
    assert(header->len == 1);
    assert(arr[0] == 42);
    array_free(arr);
}

void test_array_push(void) {
    int* arr = array_init(int);
    int item = 42;
    array_push(arr, item);
    struct array* header = array_get_header(arr);
    assert(header->len == 1);
    assert(arr[0] == 42);
    array_free(arr);
}

void test_array_resizing_dynamicly(void) {
    int* arr = array_init(int);
    for (int i = 0; i < 100; i++) {
        array_push(arr, i);
    }
    struct array* header = array_get_header(arr);
    assert(header->cap >= 100);
    assert(header->len == 100);
    for (int i = 0; i < 100; i++) {
        assert(arr[i] == i);
    }
    array_free(arr);
}

void test_array_free(void) {
    int* arr = array_init(int);
    array_push_rval(arr, 10);
    array_free(arr);
}

void test_array_cap(void) {
    int* arr             = array_init_sized(int, 50);
    struct array* header = array_get_header(arr);
    assert(array_cap(arr) == header->cap);
    assert(header->cap == 50);
    array_free(arr);
}

void test_array_len(void) {
    int* arr = array_init(int);
    assert(array_len(arr) == 0);

    array_push_rval(arr, 42);
    assert(array_len(arr) == 1);

    array_push_rval(arr, 100);
    assert(array_len(arr) == 2);

    array_free(arr);
}

void test_array_stride(void) {
    int* int_arr = array_init(int);
    assert(array_stride(int_arr) == sizeof(int));

    double* double_arr = array_init(double);
    assert(array_stride(double_arr) == sizeof(double));

    array_free(int_arr);
    array_free(double_arr);
}

void test_array_reserve(void) {
    int* array = array_init(int);
    assert(array_reserve(array, 10));
    assert(array_cap(array) == 10);
    assert(array_reserve(array, 5));
    assert(array_cap(array) == 10);
    assert(array_reserve(array, 30));
    assert(array_cap(array) == 30);

    array_free(array);
}

void test_array_resize(void) {
    int* array = array_init(int);
    assert(array_resize(array, 10));
    assert(array_cap(array) == 10);
    assert(array_resize(array, 7));
    assert(array_cap(array) == 7);
    assert(array_resize(array, 30));
    assert(array_cap(array) == 30);

    array_free(array);
}

typedef struct {
    int id;
    float value;
    char tag[8];
} test_item_t;

void test_array_remove_index(void) {
    int* array = array_init(int);

    array_push_rval(array, 12);
    assert(array_len(array) == 1);
    array_remove_index(array, 0);
    assert(array_len(array) == 0);

    array_push_rval(array, 42);
    array_push_rval(array, 52);
    array_push_rval(array, 22);
    assert(array[0] == 42);
    assert(array_len(array) == 3);

    array_remove_index(array, 0);
    assert(array_len(array) == 2);
    assert(array[0] == 52);

    array_push_rval(array, 25);
    assert(array_len(array) == 3);
    assert(array[2] == 25);
    array_remove_index(array, 0);
    assert(array[1] == 25);
    assert(array_len(array) == 2);

    array_free(array);

    test_item_t* sarr = array_init(test_item_t);
    test_item_t a     = {.id = 1, .value = 1.0f, .tag = "one"};
    test_item_t b     = {.id = 2, .value = 2.0f, .tag = "two"};
    test_item_t c     = {.id = 3, .value = 3.0f, .tag = "three"};
    array_push(sarr, a);
    array_push(sarr, b);
    array_push(sarr, c);

    array_remove_index(sarr, 1);
    assert(array_len(sarr) == 2);
    assert(sarr[0].id == 1);
    assert(sarr[1].id == 3);
    assert(sarr[1].value == 3.0f);

    array_remove_index(sarr, 0);
    assert(array_len(sarr) == 1);
    assert(sarr[0].id == 3);
    assert(sarr[0].value == 3.0f);

    array_remove_index(sarr, 0);
    assert(array_len(sarr) == 0);

    array_free(sarr);
}

int main(void) {
    START_SUITE;
    RUN(test_array_initialization);
    RUN(test_array_init_sized);
    RUN(test_array_push_rval);
    RUN(test_array_push);
    RUN(test_array_resizing_dynamicly);
    RUN(test_array_free);
    RUN(test_array_cap);
    RUN(test_array_len);
    RUN(test_array_stride);
    RUN(test_array_resize);
    RUN(test_array_reserve);
    RUN(test_array_remove_index);
    SUMMERY;
    return 0;
}
