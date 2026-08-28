#define HEADER_ARRAY_IMPLEMENTATION
#include "arrays/header_array.h"

#include "test_suite.h"

TEST_CASE(test_array_initialization, {
    int* arr = array_init(int);
    ASSERT_TRUE(arr != NULL);
    array_free(arr);
})

TEST_CASE(test_array_init_sized, {
    int* arr = array_init_sized(int, 10);
    ASSERT_TRUE(arr != NULL);
    struct array* header = array_get_header(arr);
    ASSERT_CMP(header->cap, (size_t)10);
    array_free(arr);
})

TEST_CASE(test_array_push_rval, {
    int* arr = array_init(int);
    array_push_rval(arr, 42);
    struct array* header = array_get_header(arr);
    ASSERT_CMP(header->len, (size_t)1);
    ASSERT_CMP(arr[0], 42);
    array_free(arr);
})

TEST_CASE(test_array_push, {
    int* arr = array_init(int);
    int item = 42;
    array_push(arr, item);
    struct array* header = array_get_header(arr);
    ASSERT_CMP(header->len, (size_t)1);
    ASSERT_CMP(arr[0], 42);
    array_free(arr);
})

TEST_CASE(test_array_resizing_dynamicly, {
    int* arr = array_init(int);
    for (int i = 0; i < 100; i++) {
        array_push(arr, i);
    }
    struct array* header = array_get_header(arr);
    ASSERT_TRUE(header->cap >= 100);
    ASSERT_CMP(header->len, (size_t)100);
    for (int i = 0; i < 100; i++) {
        ASSERT_CMP(arr[i], i);
    }
    array_free(arr);
})

TEST_CASE(test_array_free, {
    int* arr = array_init(int);
    array_push_rval(arr, 10);
    array_free(arr);
})

TEST_CASE(test_array_cap, {
    int* arr             = array_init_sized(int, 50);
    struct array* header = array_get_header(arr);
    ASSERT_CMP(array_cap(arr), header->cap);
    ASSERT_CMP(header->cap, (size_t)50);
    array_free(arr);
})

TEST_CASE(test_array_len, {
    int* arr = array_init(int);
    ASSERT_CMP(array_len(arr), (size_t)0);

    array_push_rval(arr, 42);
    ASSERT_CMP(array_len(arr), (size_t)1);

    array_push_rval(arr, 100);
    ASSERT_CMP(array_len(arr), (size_t)2);

    array_free(arr);
})

TEST_CASE(test_array_stride, {
    int* int_arr = array_init(int);
    ASSERT_CMP(array_stride(int_arr), sizeof(int));

    double* double_arr = array_init(double);
    ASSERT_CMP(array_stride(double_arr), sizeof(double));

    array_free(int_arr);
    array_free(double_arr);
})

TEST_CASE(test_array_reserve, {
    int* array = array_init(int);
    ASSERT_TRUE(array_reserve(array, 10));
    ASSERT_CMP(array_cap(array), (size_t)10);
    ASSERT_TRUE(array_reserve(array, 5));
    ASSERT_CMP(array_cap(array), (size_t)10);
    ASSERT_TRUE(array_reserve(array, 30));
    ASSERT_CMP(array_cap(array), (size_t)30);

    array_free(array);
})

TEST_CASE(test_array_resize, {
    int* array = array_init(int);
    ASSERT_TRUE(array_resize(array, 10));
    ASSERT_CMP(array_cap(array), (size_t)10);
    ASSERT_TRUE(array_resize(array, 7));
    ASSERT_CMP(array_cap(array), (size_t)7);
    ASSERT_TRUE(array_resize(array, 30));
    ASSERT_CMP(array_cap(array), (size_t)30);

    array_free(array);
})

typedef struct {
    int id;
    float value;
    char tag[8];
} test_item_t;

TEST_CASE(test_array_remove_index, {
    int* array = array_init(int);

    array_push_rval(array, 12);
    ASSERT_CMP(array_len(array), (size_t)1);
    array_remove_index(array, 0);
    ASSERT_CMP(array_len(array), (size_t)0);

    array_push_rval(array, 42);
    array_push_rval(array, 52);
    array_push_rval(array, 22);
    ASSERT_CMP(array[0], 42);
    ASSERT_CMP(array_len(array), (size_t)3);

    array_remove_index(array, 0);
    ASSERT_CMP(array_len(array), (size_t)2);
    ASSERT_CMP(array[0], 52);

    array_push_rval(array, 25);
    ASSERT_CMP(array_len(array), (size_t)3);
    ASSERT_CMP(array[2], 25);
    array_remove_index(array, 0);
    ASSERT_CMP(array[1], 25);
    ASSERT_CMP(array_len(array), (size_t)2);

    array_free(array);

    test_item_t* sarr = array_init(test_item_t);
    test_item_t a     = {.id = 1, .value = 1.0f, .tag = "one"};
    test_item_t b     = {.id = 2, .value = 2.0f, .tag = "two"};
    test_item_t c     = {.id = 3, .value = 3.0f, .tag = "three"};
    array_push(sarr, a);
    array_push(sarr, b);
    array_push(sarr, c);

    array_remove_index(sarr, 1);
    ASSERT_CMP(array_len(sarr), (size_t)2);
    ASSERT_CMP(sarr[0].id, 1);
    ASSERT_CMP(sarr[1].id, 3);
    ASSERT_CMP(sarr[1].value, 3.0f);

    array_remove_index(sarr, 0);
    ASSERT_CMP(array_len(sarr), (size_t)1);
    ASSERT_CMP(sarr[0].id, 3);
    ASSERT_CMP(sarr[0].value, 3.0f);

    array_remove_index(sarr, 0);
    ASSERT_CMP(array_len(sarr), (size_t)0);

    array_free(sarr);
})

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
    END_SUITE;
    return 0;
}
