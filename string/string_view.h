#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * Single-header string view library.
 *
 * A string_view is a non-owning pointer+length pair. No allocation, no
 * null-terminator required. The underlying data must outlive the view.
 *
 * Usage:
 *   #define STRING_VIEW_IMPLEMENTATION
 *   #include "string_view.h"
 *
 * Example:
 *
 *   char buf[] = "hello world";
 *   string_view sv = sv_string_to_string_view(buf);
 *   string_view word = sv_substring(sv, 0, 5);
 *   printf(SV_FMT "\n", SV_PRINT(word));  // "hello"
 */

/**
 * @brief Non-owning view into a string
 */
typedef struct {
    size_t size;
    char* data;
} string_view;

/**
 * @brief Cut off the first amount characters from the view
 *
 * @param sv Source view
 * @param amount Number of characters to skip from left
 * @return Trimmed view, or empty view if amount > sv.size
 */
string_view sv_trim_left(const string_view sv, size_t amount);

/**
 * @brief Cut off the last amount characters from the view
 *
 * @param sv Source view
 * @param amount Number of characters to remove from right
 * @return Trimmed view, or empty view if amount > sv.size
 */
string_view sv_trim_right(const string_view sv, size_t amount);

/**
 * @brief Check if view ends with given suffix
 *
 * @param sv Source view
 * @param end Suffix to check
 * @return true if sv ends with end, false otherwise
 */
bool sv_ends_with(const string_view sv, const string_view end);

/**
 * @brief Check if view starts with given prefix
 *
 * @param sv Source view
 * @param start Prefix to check
 * @return true if sv starts with start, false otherwise
 */
bool sv_starts_with(const string_view sv, const string_view start);

/**
 * @brief Lexicographically compare two views
 *
 * @param a First view
 * @param b Second view
 * @return Negative if a < b, zero if equal, positive if a > b
 */
int sv_compare(const string_view a, const string_view b);

/**
 * @brief Create view from null-terminated string
 *
 * @param str Null-terminated source string (not copied)
 * @return View over str with size = strlen(str)
 */
string_view sv_string_to_string_view(char* str);

/**
 * @brief Extract sub-view by byte range [start, end)
 *
 * @param sv Source view
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Sub-view, or empty view if range is invalid
 */
string_view sv_substring(const string_view sv, size_t start, size_t end);

/**
 * @brief Copy view contents into a buffer as null-terminated string
 *
 * @param sv Source view
 * @param dest Destination buffer
 * @param size Size of dest buffer (must be > sv.size)
 * @return true on success, false if buffer too small
 */
bool sv_copy(const string_view sv, char* dest, size_t size);

/** @brief printf format specifier for string_view */
#define SV_FMT "%.*s"
/** @brief Expand string_view for use with SV_FMT */
#define SV_PRINT(sv) (int)(sv).size, (sv).data

#ifdef STRING_VIEW_IMPLEMENTATION
string_view sv_trim_left(const string_view sv, size_t amount) {
    if (sv.size < amount) {
        return (string_view){0};
    }
    return (string_view){.size = sv.size - amount, .data = sv.data + amount};
}
string_view sv_trim_right(const string_view sv, size_t amount) {
    if (sv.size < amount) {
        return (string_view){0};
    }
    return (string_view){.size = sv.size - amount, .data = sv.data};
}

bool sv_ends_with(const string_view sv, const string_view end) {
    if (sv.size < end.size) {
        return false;
    }
    return memcmp(sv.data + sv.size - end.size, end.data, end.size) == 0;
}

bool sv_starts_with(const string_view sv, const string_view start) {
    if (sv.size < start.size) {
        return false;
    }
    return memcmp(sv.data, start.data, start.size) == 0;
}

int sv_compare(const string_view a, const string_view b) {
    size_t smaller = a.size < b.size ? a.size : b.size;
    int cmp        = memcmp(a.data, b.data, smaller);
    if (cmp != 0) return cmp;
    if (a.size < b.size) return -1;
    if (a.size > b.size) return 1;
    return 0;
}

string_view sv_string_to_string_view(char* str) { return (string_view){.size = strlen(str), .data = str}; }

string_view sv_substring(const string_view sv, size_t start, size_t end) {
    if (end > sv.size || start >= end) {
        return (string_view){0};
    }
    return (string_view){.size = end - start, .data = sv.data + start};
}

bool sv_copy(const string_view sv, char* dest, size_t size) {
    if (size <= sv.size) {
        return false;
    }
    memcpy(dest, sv.data, sv.size);
    dest[sv.size] = '\0';

    return true;
}
#endif  // STRING_VIEW_IMPLEMENTATION

#endif  // STRING_VIEW_H
