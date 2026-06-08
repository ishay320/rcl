#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
this library is about sub string of the real string
*/

typedef struct {
    size_t size;
    char* data;
} string_view;

/*
find(string_view, string_view);
*/

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

#define SV_FMT "%.*s"
#define SV_PRINT(sv) (int)(sv).size, (sv).data

#endif  // STRING_VIEW_H
