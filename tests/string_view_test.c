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

TEST_CASE(test_sv_find, {
    string_view sv = sv_string_to_string_view("hello world");

    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("hello")), (ssize_t)0, "match at start");
    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("lo wo")), (ssize_t)3, "match in middle");
    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("world")), (ssize_t)6, "match at end");
    ASSERT_CMP(sv_find(sv, sv), (ssize_t)0, "view finds itself");
    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("o")), (ssize_t)4, "single char returns first occurrence");
    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("xyz")), (ssize_t)-1, "absent needle");
    ASSERT_CMP(sv_find(sv, sv_string_to_string_view("helloX")), (ssize_t)-1, "needle with trailing mismatch");
})

TEST_CASE(test_sv_find_edge_cases, {
    string_view empty = sv_string_to_string_view("");
    string_view abab  = sv_string_to_string_view("abab");
    string_view aaab  = sv_string_to_string_view("aaab");

    ASSERT_CMP(sv_find(abab, sv_string_to_string_view("ab")), (ssize_t)0, "first of repeated occurrences");
    ASSERT_CMP(sv_find(aaab, sv_string_to_string_view("aab")), (ssize_t)1, "partial overlap before match");
    ASSERT_CMP(sv_find(abab, empty), (ssize_t)0, "empty needle matches at 0");
    ASSERT_CMP(sv_find(empty, empty), (ssize_t)0, "empty in empty");
    ASSERT_CMP(sv_find(empty, sv_string_to_string_view("a")), (ssize_t)-1, "non-empty needle in empty view");
    ASSERT_CMP(sv_find(sv_string_to_string_view("ab"), sv_string_to_string_view("abc")), (ssize_t)-1,
               "needle longer than view");
})

TEST_CASE(test_sv_find_s, {
    string_view sv = sv_string_to_string_view("hello world");

    ASSERT_CMP(sv_find_s(sv, "hello"), (ssize_t)0);
    ASSERT_CMP(sv_find_s(sv, "world"), (ssize_t)6);
    ASSERT_CMP(sv_find_s(sv, ""), (ssize_t)0, "empty C string matches at 0");
    ASSERT_CMP(sv_find_s(sv, "xyz"), (ssize_t)-1);
    ASSERT_CMP(sv_find_s(sv_string_to_string_view(""), "a"), (ssize_t)-1);
})

TEST_CASE(test_sv_chop_left, {
    string_view sv = sv_string_to_string_view("  \thello  ");

    string_view chopped = sv_chop_left(sv);
    ASSERT_CMP(chopped.size, (size_t)7, "only leading spaces and tabs are removed");
    ASSERT_CMP(strncmp(chopped.data, "hello  ", 7), 0);

    ASSERT_CMP(sv_chop_left(sv_string_to_string_view("hello")).size, (size_t)5, "nothing to chop keeps the view");
    ASSERT_CMP(sv_chop_left(sv_string_to_string_view("   ")).size, (size_t)0, "all spaces empties the view");
    ASSERT_CMP(sv_chop_left(sv_string_to_string_view("")).size, (size_t)0, "empty view stays empty");
    ASSERT_CMP(sv_chop_left(sv_string_to_string_view("\nhi")).size, (size_t)3, "newline is not chopped on the left");
})

TEST_CASE(test_sv_chop_right, {
    string_view sv = sv_string_to_string_view("  hello \t\n");

    string_view chopped = sv_chop_right(sv);
    ASSERT_CMP(chopped.size, (size_t)7, "trailing spaces, tabs and newlines are removed");
    ASSERT_CMP(strncmp(chopped.data, "  hello", 7), 0);

    ASSERT_CMP(sv_chop_right(sv_string_to_string_view("hello")).size, (size_t)5, "nothing to chop keeps the view");
    ASSERT_CMP(sv_chop_right(sv_string_to_string_view(" \t\n")).size, (size_t)0, "all whitespace empties the view");
    ASSERT_CMP(sv_chop_right(sv_string_to_string_view("")).size, (size_t)0, "empty view stays empty");
    ASSERT_CMP(sv_chop_right(sv_string_to_string_view("hi \n ")).size, (size_t)2, "stops at the first non-space");
})

TEST_CASE(test_sv_get_line, {
    string_view multi = sv_string_to_string_view("abc\ndef\n");

    string_view first = sv_get_line(multi);
    ASSERT_CMP(first.size, (size_t)3, "line ends before the newline");
    ASSERT_CMP(strncmp(first.data, "abc", 3), 0);

    ASSERT_CMP(sv_get_line(sv_string_to_string_view("abc")).size, (size_t)3, "no newline returns the whole view");
    ASSERT_CMP(sv_get_line(sv_string_to_string_view("\nabc")).size, (size_t)0, "leading newline gives empty line");
    ASSERT_CMP(sv_get_line(sv_string_to_string_view("")).size, (size_t)0, "empty view stays empty");
})
