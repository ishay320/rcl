#define STRING_BUILDER_LL_DIRECT_IMPLEMENTATION
#include "string/string_builder_ll_direct.h"

#include <assert.h>
#include <string.h>

#include "test_suite.h"

void test_sb_create(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    sb_destroy(sb);
}

void test_sb_append_single(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    assert(sb_append(sb, "hello") == true);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "hello") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_append_multiple(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    assert(sb_append(sb, "hello") == true);
    assert(sb_append(sb, ", ") == true);
    assert(sb_append(sb, "world") == true);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "hello, world") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_fappend(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    assert(sb_fappend(sb, "value=%d", 42) == true);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "value=42") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_fappend_mixed(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    assert(sb_append(sb, "hello, ") == true);
    assert(sb_fappend(sb, "world %d", 42) == true);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "hello, world 42") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_empty(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_append_empty_string(void) {
    string_builder* sb = sb_create();
    assert(sb != NULL);
    assert(sb_append(sb, "") == true);
    assert(sb_append(sb, "x") == true);
    assert(sb_append(sb, "") == true);
    const char* s = sb_get_string(sb);
    assert(s != NULL);
    assert(strcmp(s, "x") == 0);
    free((char*)s);
    sb_destroy(sb);
}

void test_sb_destroy_null(void) {
    sb_destroy(NULL);
}

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
    SUMMERY;
    return 0;
}
