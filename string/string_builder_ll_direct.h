#ifndef STRING_BUILDER_LL_DIRECT_H
#define STRING_BUILDER_LL_DIRECT_H
/*
 * String builder backed by a typed direct linked-list queue
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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../queue/queue_ll_direct.h"

/** @brief Internal typed queue node holding one appended string segment. */
QD_QUEUE_DEFINE(sb_node, char*);

/**
 * @brief String builder handle.
 *
 */
typedef struct {
    sb_node_q q;
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

#ifdef STRING_BUILDER_LL_DIRECT_IMPLEMENTATION

/** @brief Maximum formatted-string length accepted by sb_fappend() */
#define SB_BUFFER_SIZE 1024

string_builder* sb_create(void) {
    string_builder* sb = calloc(1, sizeof(*sb));
    if (sb == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
    }
    return sb;
}

bool sb_append(string_builder* s, const char* str) {
    assert(s != NULL);
    assert(str != NULL);
    size_t len      = strlen(str);
    char* new_value = malloc(len + 1);
    if (new_value == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return false;
    }
    memcpy(new_value, str, len + 1);
    bool ret = qd_push(&s->q, new_value);
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
    int r = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (r < 0) return false;
    return sb_append(s, buffer);
}

const char* sb_get_string(const string_builder* s) {
    char* result = malloc(s->size + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }
    size_t offset = 0;
    for (sb_node* node = s->q.front; node != NULL; node = node->next) {
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
    for (sb_node* node = s->q.front; node != NULL; node = node->next) {
        free(node->data);
    }
    qd_clear(&s->q);
    free(s);
}

#endif  // STRING_BUILDER_LL_DIRECT_IMPLEMENTATION
#endif  // STRING_BUILDER_LL_DIRECT_H

// int main(void) {
//     string_builder* s = sb_create();
//     if (s == NULL) return EXIT_FAILURE;

//     sb_append(s, "Hello");
//     printf("%s\n", sb_get_string(s));
//     sb_append(s, " ");
//     sb_append(s, "World!");
//     printf("%s\n", sb_get_string(s));
//     sb_fappend(s, " The answer is %d.", 42);
//     printf("%s\n", sb_get_string(s));
//     printf("%s\n", sb_get_string(s));
//     sb_destroy(s);
//     return 0;
// }
