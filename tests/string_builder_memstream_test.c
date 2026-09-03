#define STRING_BUILDER_MEMSTREAM_IMPLEMENTATION
#include "string/string_builder_memstream.h"

#include <string.h>

#include "test_suite.h"

/* sb_get_string() returns the internal buffer: never free()d here,
 * and a returned pointer is only used until the next append */

TEST_CASE(test_sb_empty_builder_yields_empty_string, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "");
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_concatenates_in_order, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    ASSERT_TRUE(sb_append(sb, ", "));
    ASSERT_TRUE(sb_fappend(sb, "world %d", 42));
    ASSERT_CMP(sb_get_string(sb), "hello, world 42");
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_formats, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_fappend(sb, "%s=%d (%05.2f)", "x", -42, 3.5));
    ASSERT_CMP(sb_get_string(sb), "x=-42 (03.50)");
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_empty_string_adds_nothing, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, ""));
    ASSERT_TRUE(sb_append(sb, "x"));
    ASSERT_TRUE(sb_append(sb, ""));
    ASSERT_CMP(sb_get_string(sb), "x");
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_has_no_length_limit, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    char input[4096 + 1];
    memset(input, 'a', sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';
    ASSERT_TRUE(sb_fappend(sb, "%s", input));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(strlen(s), (size_t)4096, "memstream fappend must not truncate long output");
    ASSERT_CMP(s, input);
    sb_destroy(sb);
})

TEST_CASE(test_sb_get_string_reflects_later_appends, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "ab"));
    ASSERT_CMP(sb_get_string(sb), "ab");
    ASSERT_TRUE(sb_append(sb, "cd"));
    ASSERT_CMP(sb_get_string(sb), "abcd", "builder keeps accepting appends after a flush");
    sb_destroy(sb);
})

TEST_CASE(test_sb_buffer_grows_across_many_appends, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    char expected[10 * 500 + 1];
    for (int i = 0; i < 500; i++) {
        ASSERT_TRUE(sb_append(sb, "0123456789"));
        memcpy(expected + 10 * i, "0123456789", 10);
    }
    expected[sizeof(expected) - 1] = '\0';
    const char* s                  = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(strlen(s), (size_t)5000);
    ASSERT_CMP(s, expected);
    sb_destroy(sb);
})

TEST_CASE(test_sb_destroy_null, { sb_destroy(NULL); })

int main(void) {
    START_SUITE;
    RUN(test_sb_empty_builder_yields_empty_string);
    RUN(test_sb_append_concatenates_in_order);
    RUN(test_sb_fappend_formats);
    RUN(test_sb_append_empty_string_adds_nothing);
    RUN(test_sb_fappend_has_no_length_limit);
    RUN(test_sb_get_string_reflects_later_appends);
    RUN(test_sb_buffer_grows_across_many_appends);
    RUN(test_sb_destroy_null);
    END_SUITE;
}
