#define STRING_BUILDER_LL_DIRECT_IMPLEMENTATION
#include "string/string_builder_ll_direct.h"

#include "test_suite.h"

TEST_CASE(test_sb_empty_builder_yields_empty_string, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_concatenates_in_order, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    ASSERT_TRUE(sb_append(sb, ", "));
    ASSERT_TRUE(sb_fappend(sb, "world %d", 42));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello, world 42");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_formats, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_fappend(sb, "%s=%d (%05.2f)", "x", -42, 3.5));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "x=-42 (03.50)");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_empty_string_adds_nothing, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, ""));
    ASSERT_TRUE(sb_append(sb, "x"));
    ASSERT_TRUE(sb_append(sb, ""));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "x");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_truncates_to_buffer_size, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    char input[SB_BUFFER_SIZE + 16];
    memset(input, 'a', sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';
    ASSERT_TRUE(sb_fappend(sb, "%s", input));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(strlen(s), (size_t)(SB_BUFFER_SIZE - 1),
               "vsnprintf keeps SB_BUFFER_SIZE - 1 chars plus the terminator");
    input[SB_BUFFER_SIZE - 1] = '\0';
    ASSERT_CMP(s, input);
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_usable_after_get_string, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "ab"));
    const char* first = sb_get_string(sb);
    ASSERT_TRUE(first != NULL);
    ASSERT_TRUE(sb_append(sb, "cd"));
    const char* second = sb_get_string(sb);
    ASSERT_TRUE(second != NULL);
    ASSERT_CMP(first, "ab", "earlier result is an independent copy");
    ASSERT_CMP(second, "abcd", "builder keeps accepting appends after get_string");
    free((char*)second);
    free((char*)first);
    sb_destroy(sb);
})

TEST_CASE(test_sb_many_appended_segments, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    char expected[3 * 100 + 1];
    for (int i = 0; i < 100; i++) {
        ASSERT_TRUE(sb_append(sb, "abc"));
        memcpy(expected + 3 * i, "abc", 3);
    }
    expected[sizeof(expected) - 1] = '\0';
    const char* s                  = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(strlen(s), (size_t)300);
    ASSERT_CMP(s, expected);
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_destroy_null, { sb_destroy(NULL); })

int main(void) {
    START_SUITE;
    RUN(test_sb_empty_builder_yields_empty_string);
    RUN(test_sb_append_concatenates_in_order);
    RUN(test_sb_fappend_formats);
    RUN(test_sb_append_empty_string_adds_nothing);
    RUN(test_sb_fappend_truncates_to_buffer_size);
    RUN(test_sb_usable_after_get_string);
    RUN(test_sb_many_appended_segments);
    RUN(test_sb_destroy_null);
    END_SUITE;
}
