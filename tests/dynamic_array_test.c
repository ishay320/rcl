#include "arrays/dynamic_array.h"

#include <limits.h>

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
typedef struct {
    int id;
    float weight;
} item_t;
typedef struct {
    size_t size;
    size_t capacity;
    item_t* data;
} da_item;

TEST_CASE(test_da_append_stores_values_in_order, {
    da_i arr   = {0};
    int vals[] = {0, -1, INT_MIN, INT_MAX, 42};
    for (int i = 0; i < 5; i++) ASSERT_TRUE(da_append(arr, vals[i]));

    ASSERT_CMP(arr.size, (size_t)5);
    for (int i = 0; i < 5; i++) ASSERT_CMP(arr.data[i], vals[i]);

    da_destroy(&arr);
})

TEST_CASE(test_da_capacity_doubles_when_full, {
    da_i arr = {0};

    da_append(arr, 0);
    ASSERT_CMP(arr.capacity, (size_t)DA_INITIAL_CAPACITY,
               "first append allocates the initial capacity");

    for (int i = 1; i < DA_INITIAL_CAPACITY; i++) da_append(arr, i);
    ASSERT_CMP(arr.capacity, (size_t)DA_INITIAL_CAPACITY, "filling up to capacity must not grow");

    da_append(arr, DA_INITIAL_CAPACITY);
    ASSERT_CMP(arr.capacity, (size_t)(2 * DA_INITIAL_CAPACITY), "append past capacity doubles it");

    for (int i = DA_INITIAL_CAPACITY + 1; i < 50; i++) da_append(arr, i);
    ASSERT_CMP(arr.size, (size_t)50);
    ASSERT_CMP(arr.capacity, (size_t)64, "8 -> 16 -> 32 -> 64");
    for (int i = 0; i < 50; i++) ASSERT_CMP(arr.data[i], i);

    da_destroy(&arr);
})

TEST_CASE(test_da_reserve_grows_but_never_shrinks, {
    da_i arr = {0};
    ASSERT_TRUE(da_reserve(arr, 10));
    ASSERT_CMP(arr.capacity, (size_t)10);
    ASSERT_CMP(arr.size, (size_t)0, "reserve must not change size");

    ASSERT_TRUE(da_reserve(arr, 4));
    ASSERT_CMP(arr.capacity, (size_t)10, "smaller reserve is a no-op");
    ASSERT_TRUE(da_reserve(arr, 10));
    ASSERT_CMP(arr.capacity, (size_t)10, "equal reserve is a no-op");

    for (int i = 0; i < 10; i++) da_append(arr, i);
    ASSERT_CMP(arr.capacity, (size_t)10, "appends within reserved space must not grow");

    da_append(arr, 10);
    ASSERT_CMP(arr.capacity, (size_t)20, "growth doubles from the reserved capacity");
    ASSERT_CMP(arr.data[10], 10);

    da_destroy(&arr);
})

TEST_CASE(test_da_destroy_zeroes_struct_and_allows_reuse, {
    da_i arr = {0};
    for (int i = 0; i < 5; i++) da_append(arr, i * 10);

    da_destroy(&arr);
    ASSERT_CMP(arr.data, NULL);
    ASSERT_CMP(arr.size, (size_t)0);
    ASSERT_CMP(arr.capacity, (size_t)0);

    da_destroy(&arr); /* destroying an already destroyed array is safe */

    for (int i = 0; i < 3; i++) da_append(arr, i + 100);
    ASSERT_CMP(arr.size, (size_t)3, "a destroyed array starts fresh");
    ASSERT_CMP(arr.data[2], 102);

    da_destroy(&arr);
})

TEST_CASE(test_da_generic_element_types, {
    da_s strs = {0};
    ASSERT_TRUE(da_append(strs, "hello"));
    ASSERT_TRUE(da_append(strs, "world"));
    ASSERT_CMP(strs.data[0], "hello");
    ASSERT_CMP(strs.data[1], "world");
    da_destroy(&strs);

    da_p ptrs = {0};
    int a = 1, b = 2;
    ASSERT_TRUE(da_append(ptrs, &a));
    ASSERT_TRUE(da_append(ptrs, &b));
    ASSERT_CMP(ptrs.data[0], (void*)&a);
    ASSERT_CMP(ptrs.data[1], (void*)&b);
    da_destroy(&ptrs);

    da_item items = {0};
    item_t first  = {.id = 1, .weight = 1.5f};
    item_t second = {.id = 2, .weight = 2.5f};
    ASSERT_TRUE(da_append(items, first));
    ASSERT_TRUE(da_append(items, second));
    ASSERT_CMP(items.data[0].id, 1);
    ASSERT_CMP(items.data[0].weight, 1.5f);
    ASSERT_CMP(items.data[1].id, 2);
    ASSERT_CMP(items.data[1].weight, 2.5f);
    da_destroy(&items);
})

int main(void) {
    START_SUITE;
    RUN(test_da_append_stores_values_in_order);
    RUN(test_da_capacity_doubles_when_full);
    RUN(test_da_reserve_grows_but_never_shrinks);
    RUN(test_da_destroy_zeroes_struct_and_allows_reuse);
    RUN(test_da_generic_element_types);
    END_SUITE;
}
