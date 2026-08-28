#define STRING_VIEW_IMPLEMENTATION
#include "string/string_view.h"

#include <string.h>

#include "test_suite.h"

TEST_CASE(test_sv_string_to_string_view, {
    char* s        = "hello";
    string_view sv = sv_string_to_string_view(s);
    ASSERT_CMP(sv.size, (size_t)5);
    ASSERT_CMP((const void*)sv.data, (const void*)s, "same pointer, not just same text");

    string_view empty = sv_string_to_string_view("");
    ASSERT_CMP(empty.size, (size_t)0);
})

TEST_CASE(test_sv_substring, {
    string_view sv = sv_string_to_string_view("hello world");

    string_view head = sv_substring(sv, 0, 5);
    ASSERT_CMP(head.size, (size_t)5);
    ASSERT_CMP(strncmp(head.data, "hello", 5), 0);

    string_view tail = sv_substring(sv, 6, 11);
    ASSERT_CMP(tail.size, (size_t)5);
    ASSERT_CMP(strncmp(tail.data, "world", 5), 0);

    string_view zero = sv_substring(sv, 3, 3);
    ASSERT_CMP(zero.size, (size_t)0);
})

TEST_CASE(test_sv_trim_left, {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_left(sv, 2);
    ASSERT_CMP(trimmed.size, (size_t)3);
    ASSERT_CMP(strncmp(trimmed.data, "llo", 3), 0);

    string_view none = sv_trim_left(sv, 0);
    ASSERT_CMP(none.size, (size_t)5);

    string_view all = sv_trim_left(sv, 5);
    ASSERT_CMP(all.size, (size_t)0);
})

TEST_CASE(test_sv_trim_right, {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_right(sv, 2);
    ASSERT_CMP(trimmed.size, (size_t)3);
    ASSERT_CMP(strncmp(trimmed.data, "hel", 3), 0);

    string_view none = sv_trim_right(sv, 0);
    ASSERT_CMP(none.size, (size_t)5);

    string_view all = sv_trim_right(sv, 5);
    ASSERT_CMP(all.size, (size_t)0);
})

TEST_CASE(test_sv_starts_with, {
    string_view sv      = sv_string_to_string_view("hello world");
    string_view match   = sv_string_to_string_view("hello");
    string_view nomatch = sv_string_to_string_view("world");
    string_view empty   = sv_string_to_string_view("");

    ASSERT_TRUE(sv_starts_with(sv, match));
    ASSERT_FALSE(sv_starts_with(sv, nomatch));
    ASSERT_TRUE(sv_starts_with(sv, empty));
    ASSERT_TRUE(sv_starts_with(sv, sv));
})

TEST_CASE(test_sv_ends_with, {
    string_view sv      = sv_string_to_string_view("hello world");
    string_view match   = sv_string_to_string_view("world");
    string_view nomatch = sv_string_to_string_view("hello");
    string_view empty   = sv_string_to_string_view("");

    ASSERT_TRUE(sv_ends_with(sv, match));
    ASSERT_FALSE(sv_ends_with(sv, nomatch));
    ASSERT_TRUE(sv_ends_with(sv, empty));
    ASSERT_TRUE(sv_ends_with(sv, sv));
})

TEST_CASE(test_sv_copy, {
    string_view sv = sv_string_to_string_view("hello");
    char buf[16]   = {0};

    ASSERT_TRUE(sv_copy(sv, buf, sizeof(buf)));
    ASSERT_CMP(strncmp(buf, "hello", 5), 0);

    char small[3] = {0};
    ASSERT_FALSE(sv_copy(sv, small, sizeof(small)));
})

TEST_CASE(test_sv_compare, {
    string_view a      = sv_string_to_string_view("abc");
    string_view same   = sv_string_to_string_view("abc");
    string_view after  = sv_string_to_string_view("abd");
    string_view short_ = sv_string_to_string_view("ab");

    ASSERT_CMP(sv_compare(a, same), 0);
    ASSERT_TRUE(sv_compare(a, after) < 0);
    ASSERT_TRUE(sv_compare(after, a) > 0);
    ASSERT_TRUE(sv_compare(short_, a) < 0);
    ASSERT_TRUE(sv_compare(a, short_) > 0);
})

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
    END_SUITE;
    return 0;
}
