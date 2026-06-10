#define STRING_VIEW_IMPLEMENTATION
#include "string/string_view.h"

#include <assert.h>
#include <string.h>

#include "test_suite.h"

void test_sv_string_to_string_view(void) {
    char* s        = "hello";
    string_view sv = sv_string_to_string_view(s);
    assert(sv.size == 5);
    assert(sv.data == s);

    string_view empty = sv_string_to_string_view("");
    assert(empty.size == 0);
}

void test_sv_substring(void) {
    string_view sv = sv_string_to_string_view("hello world");

    string_view head = sv_substring(sv, 0, 5);
    assert(head.size == 5);
    assert(strncmp(head.data, "hello", 5) == 0);

    string_view tail = sv_substring(sv, 6, 11);
    assert(tail.size == 5);
    assert(strncmp(tail.data, "world", 5) == 0);

    string_view zero = sv_substring(sv, 3, 3);
    assert(zero.size == 0);
}

void test_sv_trim_left(void) {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_left(sv, 2);
    assert(trimmed.size == 3);
    assert(strncmp(trimmed.data, "llo", 3) == 0);

    string_view none = sv_trim_left(sv, 0);
    assert(none.size == 5);

    string_view all = sv_trim_left(sv, 5);
    assert(all.size == 0);
}

void test_sv_trim_right(void) {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_right(sv, 2);
    assert(trimmed.size == 3);
    assert(strncmp(trimmed.data, "hel", 3) == 0);

    string_view none = sv_trim_right(sv, 0);
    assert(none.size == 5);

    string_view all = sv_trim_right(sv, 5);
    assert(all.size == 0);
}

void test_sv_starts_with(void) {
    string_view sv      = sv_string_to_string_view("hello world");
    string_view match   = sv_string_to_string_view("hello");
    string_view nomatch = sv_string_to_string_view("world");
    string_view empty   = sv_string_to_string_view("");

    assert(sv_starts_with(sv, match) == true);
    assert(sv_starts_with(sv, nomatch) == false);
    assert(sv_starts_with(sv, empty) == true);
    assert(sv_starts_with(sv, sv) == true);
}

void test_sv_ends_with(void) {
    string_view sv      = sv_string_to_string_view("hello world");
    string_view match   = sv_string_to_string_view("world");
    string_view nomatch = sv_string_to_string_view("hello");
    string_view empty   = sv_string_to_string_view("");

    assert(sv_ends_with(sv, match) == true);
    assert(sv_ends_with(sv, nomatch) == false);
    assert(sv_ends_with(sv, empty) == true);
    assert(sv_ends_with(sv, sv) == true);
}

void test_sv_copy(void) {
    string_view sv = sv_string_to_string_view("hello");
    char buf[16]   = {0};

    assert(sv_copy(sv, buf, sizeof(buf)) == true);
    assert(strncmp(buf, "hello", 5) == 0);

    char small[3] = {0};
    assert(sv_copy(sv, small, sizeof(small)) == false);
}

void test_sv_compare(void) {
    string_view a      = sv_string_to_string_view("abc");
    string_view same   = sv_string_to_string_view("abc");
    string_view after  = sv_string_to_string_view("abd");
    string_view short_ = sv_string_to_string_view("ab");

    assert(sv_compare(a, same) == 0);
    assert(sv_compare(a, after) < 0);
    assert(sv_compare(after, a) > 0);
    assert(sv_compare(short_, a) < 0);
    assert(sv_compare(a, short_) > 0);
}

int main(void) {
    START_SUITE;
    RUN(test_sv_string_to_string_view);
    RUN(test_sv_substring);
    RUN(test_sv_trim_left);
    RUN(test_sv_trim_right);
    RUN(test_sv_starts_with);
    RUN(test_sv_ends_with);
    RUN(test_sv_copy);
    RUN(test_sv_compare);
    SUMMERY;
    return 0;
}
