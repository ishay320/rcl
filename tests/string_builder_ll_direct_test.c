#define STRING_BUILDER_LL_DIRECT_IMPLEMENTATION
#include "string/string_builder_ll_direct.h"

#include "test_suite.h"

TEST_CASE(test_sb_create, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_single, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_multiple, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello"));
    ASSERT_TRUE(sb_append(sb, ", "));
    ASSERT_TRUE(sb_append(sb, "world"));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello, world");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_fappend(sb, "value=%d", 42));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "value=42");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_fappend_mixed, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    ASSERT_TRUE(sb_append(sb, "hello, "));
    ASSERT_TRUE(sb_fappend(sb, "world %d", 42));
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "hello, world 42");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_empty, {
    string_builder* sb = sb_create();
    ASSERT_TRUE(sb != NULL);
    const char* s = sb_get_string(sb);
    ASSERT_TRUE(s != NULL);
    ASSERT_CMP(s, "");
    free((char*)s);
    sb_destroy(sb);
})

TEST_CASE(test_sb_append_empty_string, {
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
