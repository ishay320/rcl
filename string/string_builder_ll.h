#ifndef STRING_BUILDER_LL_H
#define STRING_BUILDER_LL_H
/*
 * String builder backed by a linked-list queue
 * copy the input string to the heap and store it in a linked list node:
 *
 *   string_builder* sb = sb_create();
 *   sb_append(sb, "hello, ");
 *   sb_fappend(sb, "world %d", 42);
 *   const char* s = sb_get_string(sb);  // caller owns, must free()
 *   ...
 *   sb_destroy(sb);
 *
 * Note: this implementation is fast in the init faze but slow in the append faze,
 * because it allocates a new node for each appended string segment.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @brief Internal linked-list node that holds one appended string segment. */
typedef struct sb_ll {
    char* data;
    struct sb_ll* next;
} sb_ll;

/**
 * @brief String builder handle.
 *
 */
typedef struct {
    sb_ll* head;
    size_t size;
} string_builder;

/**
 * @brief Allocate and initialise a new string builder.
 *
 * @return Pointer to a new string_builder, or NULL on allocation failure.
 */
string_builder* sb_create(void);

/**
 * @brief Append a copy of a string to the builder.
 *
 * @param s   The string builder.
 * @param str Null-terminated string to append.
 * @return true on success, false on allocation failure.
 */
bool sb_append(string_builder* s, const char* str);

/**
 * @brief Append a printf-style formatted string to the builder.
 *
 * @param s      The string builder.
 * @param format printf format string.
 * @param ...    Format arguments.
 * @return true on success, false if formatting fails or on allocation failure.
 * @note Result is truncated to SB_BUFFER_SIZE characters.
 */
bool sb_fappend(string_builder* s, const char* format, ...);

/**
 * @brief Build and return the concatenated string.
 *
 * @param s The string builder.
 * @return Heap-allocated null-terminated string containing all appended
 *         segments, or NULL on allocation failure. Caller must free().
 */
const char* sb_get_string(const string_builder* s);

/**
 * @brief Free all resources owned by the string builder.
 *
 * @param s The string builder to destroy. No-op if NULL.
 */
void sb_destroy(string_builder* s);

#ifdef STRING_BUILDER_LL_IMPLEMENTATION

#include "../queue/queue_ll.h"

/** @brief Maximum formatted-string length accepted by sb_fappend(). */
#define SB_BUFFER_SIZE 1024

string_builder* sb_create(void) {
    string_builder* sb = malloc(sizeof(*sb));
    if (sb == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }
    sb->size = 0;
    sb->head = qe_init_header(sb_ll);
    if (sb->head == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        free(sb);
        return NULL;
    }
    return sb;
}

bool sb_append(string_builder* s, const char* str) {
    assert(s != NULL);
    assert(str != NULL);
    size_t len      = strlen(str);
    char* new_value = malloc(sizeof(*new_value) * (len + 1));
    if (new_value == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return false;
    }
    memcpy(new_value, str, len + 1);

    bool ret = qe_push(s->head, new_value);
    if (ret) {
        s->size += len;
    } else {
        free(new_value);
    }
    return ret;
}

bool sb_fappend(string_builder* s, const char* format, ...) {
    assert(s != NULL);
    assert(format != NULL);

    char buffer[SB_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    if (vsnprintf(buffer, sizeof(buffer), format, args) < 0) {
        va_end(args);
        return false;
    }
    va_end(args);
    return sb_append(s, buffer);
}

const char* sb_get_string(const string_builder* s) {
    char* result = malloc(sizeof(*result) * (s->size + 1));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }
    size_t offset = 0;
    qe_for_each(s->head, node) {
        assert(node->data != NULL);
        size_t len = strlen(node->data);
        memcpy(result + offset, node->data, len);
        offset += len;
    }
    result[offset] = '\0';
    return result;
}

void sb_destroy(string_builder* s) {
    if (s == NULL) return;
    qe_for_each(s->head, node) {
        free(node->data);
    }
    qe_destroy_header(s->head);
    free(s);
}

#endif  // STRING_BUILDER_LL_IMPLEMENTATION
#endif  // STRING_BUILDER_LL_H
