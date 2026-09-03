#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Generic dynamic array via macros.
 * Define an empty struct with size, capacity, data and use:
 *   da_append(arr, item)  -- append, returns bool (false on alloc fail)
 *   da_reserve(arr, n)    -- ensure capacity >= n, returns bool
 *   da_destroy(&arr)      -- free and zero the struct
 *
 * Example:
 *   typedef struct { size_t size; size_t capacity; int*   data; } da_i;
 *   typedef struct { size_t size; size_t capacity; void** data; } da_p;
 */

/* initial capacity; doubles on each resize */
#define DA_INITIAL_CAPACITY 8

/**
 * @brief Append an item to the dynamic array
 *
 * @param array The dynamic array
 * @param item The item to append
 * @return true if the item was appended successfully, false on allocation failure
 */
#define da_append(array, item)                                                                     \
    ({                                                                                             \
        bool _ok = true;                                                                           \
        if ((array).size >= (array).capacity) {                                                    \
            size_t _new_cap  = (array).capacity == 0 ? DA_INITIAL_CAPACITY : (array).capacity * 2; \
            __auto_type _tmp = realloc((array).data, _new_cap * sizeof(*(array).data));            \
            if (!_tmp) {                                                                           \
                fprintf(stderr, "da_append: realloc failed\n");                                    \
                _ok = false;                                                                       \
            } else {                                                                               \
                (array).data     = _tmp;                                                           \
                (array).capacity = _new_cap;                                                       \
            }                                                                                      \
        }                                                                                          \
        if (_ok) (array).data[(array).size++] = (item);                                            \
        _ok;                                                                                       \
    })

/**
 * @brief Ensure the dynamic array has at least the specified capacity
 *
 * @param array The dynamic array
 * @param n The minimum capacity to ensure
 * @return true if the capacity was successfully ensured, false on allocation failure
 */
#define da_reserve(array, n)                                                       \
    ({                                                                             \
        bool _ok = true;                                                           \
        if ((array).capacity < (size_t)(n)) {                                      \
            __auto_type _tmp = realloc((array).data, (n) * sizeof(*(array).data)); \
            if (!_tmp) {                                                           \
                fprintf(stderr, "da_reserve: realloc failed\n");                   \
                _ok = false;                                                       \
            } else {                                                               \
                (array).data     = _tmp;                                           \
                (array).capacity = (n);                                            \
            }                                                                      \
        }                                                                          \
        _ok;                                                                       \
    })

/**
 * @brief Destroy the dynamic array and free its memory
 *
 * @param array The dynamic array to destroy
 */
#define da_destroy(array)             \
    do {                              \
        if ((array)->data) {          \
            free((array)->data);      \
            (array)->data     = NULL; \
            (array)->size     = 0;    \
            (array)->capacity = 0;    \
        }                             \
    } while (0)

#endif  // DYNAMIC_ARRAY_H