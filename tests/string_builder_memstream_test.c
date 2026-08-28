#define STRING_BUILDER_MEMSTREAM_IMPLEMENTATION
#include "string/string_builder_memstream.h"

#include "test_suite.h"

TEST_CASE(test_sb_create, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_single, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello");
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_multiple, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    ASSERT_TRUE(sb_append(sb, ", "));
    ASSERT_TRUE(sb_append(sb, "world"));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello, world");
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_fappend(sb, "value=%d", 42));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "value=42");
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_mixed, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello, "));
    ASSERT_TRUE(sb_fappend(sb, "world %d", 42));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello, world 42");
    sb_destroy(sb);
})

TEST_CASE(test_sb_empty, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "");
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_empty_string, {
    sb_memstream* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, ""));
    ASSERT_TRUE(sb_append(sb, "x"));
    ASSERT_TRUE(sb_append(sb, ""));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "x");
    sb_destroy(sb);
})

TEST_CASE(test_sb_destroy_null, { sb_destroy(NULL); })

int main(void) {
    START_SUITE;
    RUN(test_sb_create);
    RUN(test_sb_append_single);
    RUN(test_sb_append_multiple);
    RUN(test_sb_fappend);
    RUN(test_sb_fappend_mixed);
    RUN(test_sb_empty);
    RUN(test_sb_append_empty_string);
    RUN(test_sb_destroy_null);
    END_SUITE;
    return 0;
}
