#ifndef HEADER_ARRAY_H
#define HEADER_ARRAY_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Single-header dynamic array library.
 *
 * A type-safe growable array. The returned pointer is the data buffer directly,
 * so `arr[i]` works as expected. Metadata lives in a hidden header before the pointer.
 *
 * Usage:
 *   #define HEADER_ARRAY_IMPLEMENTATION
 *   #include "header_array.h"
 *
 * Example:
 *
 *   int* arr = array_init(int);
 *   array_push_rval(arr, 42);
 *   printf("%d\n", arr[0]);
 *   array_free(arr);
 */

/**
 * @brief Internal header stored before the data pointer.
 *
 * Users never access this directly; use the macro API instead.
 */
struct array {
    size_t len;    /**< Number of elements currently stored. */
    size_t cap;    /**< Allocated capacity in elements. */
    size_t stride; /**< Size in bytes of each element. */

    uint8_t data[]; /**< Data buffer; the pointer returned to the user points here. */
};

#ifndef RCL_INLINE
#define RCL_INLINE static inline
#endif

#define ARRAY_HEADER_SIZE (sizeof(size_t) * 3)

/**
 * @brief Initialise a dynamic array for the given element type.
 *
 * @param type Element type (e.g. int, MyStruct).
 * @return type* pointer to the new array, or NULL on allocation failure.
 */
#define array_init(type) (type*)f_array_init(sizeof(type), 1)

/**
 * @brief Initialise a dynamic array with a pre-allocated capacity.
 *
 * @param type Element type (e.g. int, MyStruct).
 * @param size Initial capacity in number of elements.
 * @return type* pointer to the new array, or NULL on allocation failure.
 */
#define array_init_sized(type, size) (type*)f_array_init(sizeof(type), size)

/**
 * @brief Free the dynamic array.
 *
 * @param array Pointer to the array.
 */
RCL_INLINE void array_free(void* array);

/**
 * @brief Push an lvalue element onto the array.
 *
 * @param array Array to push to.
 * @param element Lvalue element to push (address is taken internally).
 * @return bool true on success, false on allocation failure.
 */
#define array_push(array, element) f_array_push((void**)&array, &element)

/**
 * @brief Push an rvalue element onto the array.
 *
 * Accepts temporaries and literals (e.g. `array_push_rval(arr, 1 + 3)`).
 *
 * @param array Array to push to.
 * @param element Rvalue element to push.
 * @return bool true on success, false on allocation failure.
 */
#define array_push_rval(array, element)          \
    ({                                           \
        typeof(element) __tmp_value = (element); \
        array_push((array), __tmp_value);        \
    })

/** @internal Recover the array header from a data pointer. */
#define array_get_header(a) ((struct array*)((uint8_t*)(a) - ARRAY_HEADER_SIZE))

/**
 * @brief Reserve capacity without changing length.
 *
 * No-op if current capacity already meets or exceeds `reserve`.
 *
 * @param array Array to reserve space in.
 * @param reserve Minimum capacity in number of elements.
 * @return bool true on success, false on allocation failure.
 */
#define array_reserve(array, reserve) f_array_reserve((void**)(&array), reserve)

/**
 * @brief Resize the array capacity to an exact size.
 *
 * Can grow or shrink. If shrinking below current length, length is clamped to new capacity.
 *
 * @param array Array to resize.
 * @param resize New capacity in number of elements.
 * @return bool true on success, false on allocation failure.
 */
#define array_resize(array, resize) f_array_resize((void**)(&array), resize)

RCL_INLINE void* f_array_init(size_t stride, size_t cap);

RCL_INLINE bool f_array_resize(void** array, size_t size);

RCL_INLINE bool f_array_reserve(void** array, size_t size);

RCL_INLINE bool f_array_push(void** array, void* element);

/**
 * @brief Remove an element by index (order-preserving via memmove).
 *
 * @param array Pointer to the array.
 * @param index Index of the element to remove. Must be < array_len(array).
 */
RCL_INLINE void array_remove_index(void* array, size_t index);

/**
 * @brief Get the allocated capacity of the array.
 *
 * @param array Pointer to the array.
 * @return Capacity in number of elements.
 */
RCL_INLINE size_t array_cap(void* array);

/**
 * @brief Get the number of elements currently in the array.
 *
 * @param array Pointer to the array.
 * @return Number of elements stored.
 */
RCL_INLINE size_t array_len(void* array);

/**
 * @brief Get the size in bytes of each element.
 *
 * @param array Pointer to the array.
 * @return Element size in bytes.
 */
RCL_INLINE size_t array_stride(void* array);

#ifdef HEADER_ARRAY_IMPLEMENTATION

RCL_INLINE void* f_array_init(size_t stride, size_t cap) {
    struct array* header = malloc(ARRAY_HEADER_SIZE + (stride * cap));
    if (!header) {
        perror("could not create dynamic array");
        return NULL;
    }

    header->cap    = cap;
    header->len    = 0;
    header->stride = stride;

    return header->data;
}

RCL_INLINE void array_free(void* array) {
    assert(array);
    struct array* header = array_get_header(array);
    free(header);
}

RCL_INLINE bool f_array_resize(void** array, size_t size) {
    assert(*array);
    struct array* header = array_get_header(*array);
    void* new_ptr        = realloc(header, size * header->stride + ARRAY_HEADER_SIZE);
    if (!new_ptr) {
        perror("could not resize array");
        return false;
    }

    header      = new_ptr;
    header->cap = size;
    if (header->cap < header->len) {
        header->len = header->cap;
    }

    // update the outside ptr to the new position
    *array = header->data;
    return true;
}

RCL_INLINE bool f_array_reserve(void** array, size_t size) {
    assert(*array);
    struct array* header = array_get_header(*array);
    if (header->cap >= size) {
        return true;
    }

    return f_array_resize(array, size);
}

RCL_INLINE bool f_array_push(void** array, void* element) {
    assert(*array);
    struct array* header = array_get_header(*array);
    if (__builtin_expect(header->len == header->cap, 0)) {
        size_t new_cap = header->cap * 2;
        if (!f_array_resize(array, new_cap)) {
            return false;
        }
        header = array_get_header(*array);
    }

    __builtin_memcpy((uint8_t*)*array + header->stride * header->len, element, header->stride);
    header->len++;

    return true;
}

RCL_INLINE void array_remove_index(void* array, size_t index) {
    assert(array);
    struct array* header = array_get_header(array);
    assert(header->len > index);

    memmove(header->data + (index * header->stride), header->data + ((index + 1) * header->stride), (header->len - 1 - index) * header->stride);
    header->len--;
}

RCL_INLINE size_t array_cap(void* array) {
    assert(array);
    struct array* header = array_get_header(array);
    return header->cap;
}

RCL_INLINE size_t array_len(void* array) {
    assert(array);
    struct array* header = array_get_header(array);
    return header->len;
}

RCL_INLINE size_t array_stride(void* array) {
    assert(array);
    struct array* header = array_get_header(array);
    return header->stride;
}

#endif  // HEADER_ARRAY_IMPLEMENTATION
#endif  // HEADER_ARRAY_H
