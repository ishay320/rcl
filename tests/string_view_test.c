#define STRING_VIEW_IMPLEMENTATION
#include "string/string_view.h"

#include <string.h>

#include "test_suite.h"

/* note: string_view data is not null-terminated, so raw view contents are
 * checked with strncmp/sv_compare instead of ASSERT_CMP on the data pointer */

TEST_CASE(test_sv_string_to_string_view, {
    char* s        = "hello";
    string_view sv = sv_string_to_string_view(s);
    ASSERT_CMP(sv.size, (size_t)5);
    ASSERT_CMP((const void*)sv.data, (const void*)s, "view must alias the string, not copy it");

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

    string_view full = sv_substring(sv, 0, sv.size);
    ASSERT_CMP(sv_compare(full, sv), 0, "[0, size) must give back the whole view");
})

TEST_CASE(test_sv_substring_invalid_range_is_empty, {
    string_view sv = sv_string_to_string_view("hello");
    ASSERT_CMP(sv_substring(sv, 3, 3).size, (size_t)0, "start == end");
    ASSERT_CMP(sv_substring(sv, 4, 2).size, (size_t)0, "start > end");
    ASSERT_CMP(sv_substring(sv, 0, 6).size, (size_t)0, "end > size");
})

TEST_CASE(test_sv_trim_left, {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_left(sv, 2);
    ASSERT_CMP(trimmed.size, (size_t)3);
    ASSERT_CMP(strncmp(trimmed.data, "llo", 3), 0);

    ASSERT_CMP(sv_trim_left(sv, 0).size, (size_t)5, "trim of 0 keeps the view");
    ASSERT_CMP(sv_trim_left(sv, 5).size, (size_t)0, "trim of size empties the view");
    ASSERT_CMP(sv_trim_left(sv, 6).size, (size_t)0, "trim past the end is empty, not UB");
})

TEST_CASE(test_sv_trim_right, {
    string_view sv = sv_string_to_string_view("hello");

    string_view trimmed = sv_trim_right(sv, 2);
    ASSERT_CMP(trimmed.size, (size_t)3);
    ASSERT_CMP(strncmp(trimmed.data, "hel", 3), 0);

    ASSERT_CMP(sv_trim_right(sv, 0).size, (size_t)5, "trim of 0 keeps the view");
    ASSERT_CMP(sv_trim_right(sv, 5).size, (size_t)0, "trim of size empties the view");
    ASSERT_CMP(sv_trim_right(sv, 6).size, (size_t)0, "trim past the end is empty, not UB");
})

TEST_CASE(test_sv_starts_with, {
    string_view sv = sv_string_to_string_view("hello world");

    ASSERT_TRUE(sv_starts_with(sv, sv_string_to_string_view("hello")));
    ASSERT_FALSE(sv_starts_with(sv, sv_string_to_string_view("world")));
    ASSERT_TRUE(sv_starts_with(sv, sv_string_to_string_view("")), "empty prefix always matches");
    ASSERT_TRUE(sv_starts_with(sv, sv), "view starts with itself");
    ASSERT_FALSE(sv_starts_with(sv_string_to_string_view("he"), sv),
                 "prefix longer than the view can never match");
})

TEST_CASE(test_sv_ends_with, {
    string_view sv = sv_string_to_string_view("hello world");

    ASSERT_TRUE(sv_ends_with(sv, sv_string_to_string_view("world")));
    ASSERT_FALSE(sv_ends_with(sv, sv_string_to_string_view("hello")));
    ASSERT_TRUE(sv_ends_with(sv, sv_string_to_string_view("")), "empty suffix always matches");
    ASSERT_TRUE(sv_ends_with(sv, sv), "view ends with itself");
    ASSERT_FALSE(sv_ends_with(sv_string_to_string_view("ld"), sv),
                 "suffix longer than the view can never match");
})

TEST_CASE(test_sv_copy, {
    string_view sv = sv_string_to_string_view("hello");

    char buf[8];
    memset(buf, 'x', sizeof(buf));
    ASSERT_TRUE(sv_copy(sv, buf, sizeof(buf)));
    ASSERT_CMP(buf, "hello", "copy must be null-terminated");

    char fit[6];
    ASSERT_TRUE(sv_copy(sv, fit, sizeof(fit)), "size + 1 is exactly enough");
    ASSERT_CMP(fit, "hello");

    char small[5];
    ASSERT_FALSE(sv_copy(sv, small, sizeof(small)),
                 "buffer == view size leaves no room for the terminator");
})

TEST_CASE(test_sv_compare, {
    string_view abc   = sv_string_to_string_view("abc");
    string_view abd   = sv_string_to_string_view("abd");
    string_view ab    = sv_string_to_string_view("ab");
    string_view empty = sv_string_to_string_view("");

    ASSERT_CMP(sv_compare(abc, sv_string_to_string_view("abc")), 0);
    ASSERT_TRUE(sv_compare(abc, abd) < 0);
    ASSERT_TRUE(sv_compare(abd, abc) > 0);
    ASSERT_TRUE(sv_compare(ab, abc) < 0, "shared prefix: shorter view sorts first");
    ASSERT_TRUE(sv_compare(abc, ab) > 0);
    ASSERT_CMP(sv_compare(empty, empty), 0);
    ASSERT_TRUE(sv_compare(empty, abc) < 0);
})
