#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ansi.h"

static clock_t s_cpu_start = 0;
static struct timespec s_wall_start;
static size_t s_tests_run    = 0;
static size_t s_tests_failed = 0;

/*
 * ASSERT_CMP compares a and b
 * ASSERT_FALSE check if its false
 * ASSERT_TRUE check if its true
 * ASSERT_NEAR compares a and b with epsilon
 * message is optional everywhere: ASSERT_CMP(x, y) or ASSERT_CMP(x, y, "why")
 */

#define TS_DEFINE_CMP_FN(name, type, fmt)                                           \
    static inline bool name(type a, type b, char* reason_out, size_t reason_size) { \
        if (a == b) {                                                               \
            return true;                                                            \
        }                                                                           \
        snprintf(reason_out, reason_size, "got " fmt " instead of " fmt, a, b);     \
        return false;                                                               \
    }

TS_DEFINE_CMP_FN(ts_cmp_chr, char, "'%c'")
TS_DEFINE_CMP_FN(ts_cmp_int, int, "%d")
TS_DEFINE_CMP_FN(ts_cmp_uint, unsigned int, "%u")
TS_DEFINE_CMP_FN(ts_cmp_long, long, "%ld")
TS_DEFINE_CMP_FN(ts_cmp_ulong, unsigned long, "%lu")
TS_DEFINE_CMP_FN(ts_cmp_llong, long long, "%lld")
TS_DEFINE_CMP_FN(ts_cmp_ullong, unsigned long long, "%llu")
TS_DEFINE_CMP_FN(ts_cmp_ptr, const void*, "%p")

/* exact float compare on purpose - use ASSERT_NEAR when you want epsilon */
TS_DEFINE_CMP_FN(ts_cmp_flt, float, "%f")
TS_DEFINE_CMP_FN(ts_cmp_dbl, double, "%f")
#undef TS_DEFINE_CMP_FN

static inline bool ts_cmp_bool(bool a, bool b, char* reason_out, size_t reason_size) {
    if (a == b) {
        return true;
    }
    snprintf(reason_out, reason_size, "got %s instead of %s", a ? "true" : "false",
             b ? "true" : "false");
    return false;
}

static inline bool ts_cmp_str(const char* a, const char* b, char* reason_out, size_t reason_size) {
    if (a == b || (a != NULL && b != NULL && strcmp(a, b) == 0)) {
        return true;
    }
    snprintf(reason_out, reason_size, "got \"%s\" instead of \"%s\"", a ? a : "(null)",
             b ? b : "(null)");
    return false;
}

#define TS_CMP_FN(a)                       \
    _Generic((a),                          \
        bool: ts_cmp_bool,                 \
        char: ts_cmp_chr,                  \
        signed char: ts_cmp_chr,           \
        unsigned char: ts_cmp_chr,         \
        short: ts_cmp_int,                 \
        unsigned short: ts_cmp_int,        \
        int: ts_cmp_int,                   \
        unsigned int: ts_cmp_uint,         \
        long: ts_cmp_long,                 \
        unsigned long: ts_cmp_ulong,       \
        long long: ts_cmp_llong,           \
        unsigned long long: ts_cmp_ullong, \
        float: ts_cmp_flt,                 \
        double: ts_cmp_dbl,                \
        char*: ts_cmp_str,                 \
        const char*: ts_cmp_str,           \
        void*: ts_cmp_ptr,                 \
        const void*: ts_cmp_ptr,           \
        default: ts_cmp_ptr) /* any other pointer type */

#define TS_SAME_CMP_TYPE(a, b) \
    __builtin_types_compatible_p(__typeof__(TS_CMP_FN(a)), __typeof__(TS_CMP_FN(b)))

#define TS_RETURN_TEST_RESULT(_message, _reason)                                        \
    do {                                                                                \
        struct test_result tr = {                                                       \
            .failed = true, .line = __LINE__, .file = __FILE__, .message = (_message)}; \
        snprintf(tr.reason, sizeof(tr.reason), "%s", _reason);                          \
        snprintf(tr.function, sizeof(tr.function), "%s", __FUNCTION__);                 \
        return tr;                                                                      \
    } while (0)

#define ASSERT_CMP(...) ASSERT_CMP_IMPL(__VA_ARGS__, "", )
#define ASSERT_CMP_IMPL(a, b, message, ...)                                   \
    do {                                                                      \
        _Static_assert(TS_SAME_CMP_TYPE(a, b),                                \
                       "ASSERT_CMP: both arguments must have the same type"); \
        char reason[128];                                                     \
        if (!TS_CMP_FN(a)((a), (b), reason, sizeof(reason))) {                \
            TS_RETURN_TEST_RESULT(message, reason);                           \
        }                                                                     \
    } while (0)

#define ASSERT_NEAR(...) ASSERT_NEAR_IMPL(__VA_ARGS__, "", )
#define ASSERT_NEAR_IMPL(a, b, eps, _message, ...)                                                \
    do {                                                                                          \
        double _a    = (double)(a);                                                               \
        double _b    = (double)(b);                                                               \
        double _diff = _a > _b ? _a - _b : _b - _a; /* no math.h, no -lm */                       \
        if (!(_diff <= (double)(eps))) {                                                          \
            char buff[128];                                                                       \
            snprintf(buff, sizeof(buff), "got %f instead of %f (eps %f)", _a, _b, (double)(eps)); \
            TS_RETURN_TEST_RESULT(_message, buff);                                                \
        }                                                                                         \
    } while (0)

#define ASSERT_TRUE(...) ASSERT_TRUE_IMPL(__VA_ARGS__, "", )
#define ASSERT_TRUE_IMPL(result, _message, ...)                              \
    do {                                                                     \
        if (!(result)) {                                                     \
            TS_RETURN_TEST_RESULT(_message, "should be true but got false"); \
        }                                                                    \
    } while (0)

#define ASSERT_FALSE(...) ASSERT_FALSE_IMPL(__VA_ARGS__, "", )
#define ASSERT_FALSE_IMPL(result, _message, ...)                             \
    do {                                                                     \
        if ((result)) {                                                      \
            TS_RETURN_TEST_RESULT(_message, "should be false but got true"); \
        }                                                                    \
    } while (0)

struct test_result {
    bool failed;
    int line;
    char* file;
    char function[64];
    char reason[128];
    char* message;
};

void ts_print_success(const char* test_name) {
    double ms_cpu = (double)(clock() - s_cpu_start) * 1000.0 / CLOCKS_PER_SEC;
    double ms_wall;
    struct timespec wall_end;
    clock_gettime(CLOCK_MONOTONIC, &wall_end);
    ms_wall = (double)(wall_end.tv_sec - s_wall_start.tv_sec) * 1000.0 +
              (double)(wall_end.tv_nsec - s_wall_start.tv_nsec) / 1e6;
    s_tests_run++;
    printf(AN_GREEN "[%ld] '%s' passed." AN_RESET AN_DIM
                    " (cpu: %.3f ms, wall: %.3f ms)\n" AN_RESET,
           s_tests_run, test_name, ms_cpu, ms_wall);
    fflush(stdout);
}

void ts_print_failure(const struct test_result tr) {
    double ms_cpu = (double)(clock() - s_cpu_start) * 1000.0 / CLOCKS_PER_SEC;
    double ms_wall;
    struct timespec wall_end;
    clock_gettime(CLOCK_MONOTONIC, &wall_end);
    ms_wall = (double)(wall_end.tv_sec - s_wall_start.tv_sec) * 1000.0 +
              (double)(wall_end.tv_nsec - s_wall_start.tv_nsec) / 1e6;
    s_tests_run++;
    s_tests_failed++;
    printf(AN_RED "[%ld] %s:%d '%s' failed: %s -" AN_RESET " %s" AN_DIM
                  " (cpu: %.3f ms, wall: %.3f ms)\n" AN_RESET,
           s_tests_run, tr.file, tr.line, tr.function, tr.reason, tr.message, ms_cpu, ms_wall);
    fflush(stdout);
}

#define RUN(fn, name)                                  \
    do {                                               \
        s_cpu_start = clock();                         \
        clock_gettime(CLOCK_MONOTONIC, &s_wall_start); \
        struct test_result tr = fn();                  \
        if (tr.failed) {                               \
            ts_print_failure(tr);                      \
        } else {                                       \
            ts_print_success(name);                    \
        }                                              \
    } while (0)

#define START_SUITE                              \
    clock_t suite_start = clock();               \
    struct timespec wall_start;                  \
    clock_gettime(CLOCK_MONOTONIC, &wall_start); \
    printf(AN_BOLD AN_CYAN "Running " __FILE__ " tests..." AN_RESET "\n")

#define END_SUITE                                                                                \
    do {                                                                                         \
        double _total_ms_cpu = (double)(clock() - suite_start) * 1000.0 / CLOCKS_PER_SEC;        \
        struct timespec wall_end;                                                                \
        clock_gettime(CLOCK_MONOTONIC, &wall_end);                                               \
        double _total_ms_wall = (double)(wall_end.tv_sec - wall_start.tv_sec) * 1000.0 +         \
                                (double)(wall_end.tv_nsec - wall_start.tv_nsec) / 1e6;           \
        printf(AN_GREEN "\n%ld/%ld " __FILE__ " tests passed!" AN_RESET AN_DIM                   \
                        " (cpu: %.3f ms, wall: %.3f ms)\n" AN_RESET,                             \
               s_tests_run - s_tests_failed, s_tests_run, _total_ms_cpu, _total_ms_wall);        \
        if (s_tests_failed > 0) printf(AN_RED "%ld tests failed" AN_RESET "\n", s_tests_failed); \
        return SUITE_EXIT_CODE;                                                                  \
    } while (0)

/* return value for main: 0 when every test passed, 1 otherwise */
#define SUITE_EXIT_CODE (s_tests_failed == 0 ? 0 : 1)

// auto list the tests and main
struct tc_node {
    const char* t_name;
    struct test_result (*t_func)(void);
    struct tc_node* next;
};

static struct tc_node* tc_head = NULL;
static struct tc_node* tc_tail = NULL;

void tc_ll_append(struct tc_node* node) {
    if (tc_head == NULL) {
        tc_head = node;
        tc_tail = node;
        return;
    }
    tc_tail->next = node;
    tc_tail       = node;
}

#undef TEST_CASE
#define TEST_CASE(test_name, ...)                                                      \
    struct test_result test_name(void) { __VA_ARGS__ return (struct test_result){0}; } \
    __attribute__((constructor)) static void tc_##test_name(void) {                    \
        struct tc_node* n = malloc(sizeof(*n));                                        \
        n->t_func         = test_name;                                                 \
        n->t_name         = #test_name;                                                \
        n->next           = NULL;                                                      \
        tc_ll_append(n);                                                               \
    }

int main(void) {
    START_SUITE;
    for (struct tc_node* p = tc_head; p != NULL; p = p->next) {
        RUN(p->t_func, p->t_name);
    }

    struct tc_node* next = tc_head;
    while (next != NULL) {
        struct tc_node* prev = next;
        next                 = next->next;
        free(prev);
    }

    END_SUITE;
}

#endif  // TEST_SUITE_H