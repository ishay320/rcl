#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ansi.h"

static clock_t s_cpu_start = 0;
struct timespec s_wall_start;
static size_t s_tests_run = 0;

/*
 * ASSERT_CMP compares a and b
 * message is optional everywhere: ASSERT_CMP(x, y) or ASSERT_CMP(x, y, "why")
 */

#define TS_FAIL_HEADER(file, line, func, message)                                                \
    do {                                                                                         \
        if ((message) && (message)[0] != '\0') {                                                 \
            printf(AN_RED "%s %s:%d FAIL:" AN_RESET " %s: ", (func), (file), (line), (message)); \
        } else {                                                                                 \
            printf(AN_RED "%s %s:%d FAIL:" AN_RESET " ", (func), (file), (line));                \
        }                                                                                        \
    } while (0)

#define TS_DEFINE_CMP_FN(name, type, fmt)                                                                        \
    static inline bool name(type a, type b, const char* message, const char* file, int line, const char* func) { \
        if (a == b) {                                                                                            \
            return true;                                                                                         \
        }                                                                                                        \
        TS_FAIL_HEADER(file, line, func, message);                                                               \
        printf("got " fmt " instead of " fmt "\n", a, b);                                                        \
        return false;                                                                                            \
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

static inline bool ts_cmp_bool(bool a, bool b, const char* message, const char* file, int line, const char* func) {
    if (a == b) {
        return true;
    }
    TS_FAIL_HEADER(file, line, func, message);
    printf("got %s instead of %s\n", a ? "true" : "false", b ? "true" : "false");
    return false;
}

static inline bool ts_cmp_str(const char* a, const char* b, const char* message, const char* file, int line, const char* func) {
    if (a == b || (a != NULL && b != NULL && strcmp(a, b) == 0)) {
        return true;
    }
    TS_FAIL_HEADER(file, line, func, message);
    printf("got \"%s\" instead of \"%s\"\n", a ? a : "(null)", b ? b : "(null)");
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

#define TS_SAME_CMP_TYPE(a, b) __builtin_types_compatible_p(__typeof__(TS_CMP_FN(a)), __typeof__(TS_CMP_FN(b)))

#define ASSERT_CMP(...) ASSERT_CMP_IMPL(__VA_ARGS__, "", )
#define ASSERT_CMP_IMPL(a, b, message, ...)                                                           \
    do {                                                                                              \
        _Static_assert(TS_SAME_CMP_TYPE(a, b), "ASSERT_CMP: both arguments must have the same type"); \
        if (!TS_CMP_FN(a)((a), (b), (message), __FILE__, __LINE__, __FUNCTION__)) {                   \
            fflush(stdout);                                                                           \
            abort();                                                                                  \
        }                                                                                             \
    } while (0)

#define ASSERT_NEAR(...) ASSERT_NEAR_IMPL(__VA_ARGS__, "", )
#define ASSERT_NEAR_IMPL(a, b, eps, message, ...)                             \
    do {                                                                      \
        double _a    = (double)(a);                                           \
        double _b    = (double)(b);                                           \
        double _diff = _a > _b ? _a - _b : _b - _a; /* no math.h, no -lm */   \
        if (!(_diff <= (double)(eps))) {                                      \
            TS_FAIL_HEADER(__FILE__, __LINE__, __FUNCTION__, (message));      \
            printf("got %f instead of %f (eps %f)\n", _a, _b, (double)(eps)); \
            fflush(stdout);                                                   \
            abort();                                                          \
        }                                                                     \
    } while (0)

#define ASSERT_TRUE(...) ASSERT_TRUE_IMPL(__VA_ARGS__, "", )
#define ASSERT_TRUE_IMPL(result, message, ...)                           \
    do {                                                                 \
        if (!(result)) {                                                 \
            TS_FAIL_HEADER(__FILE__, __LINE__, __FUNCTION__, (message)); \
            printf("should be true but got false\n");                    \
            fflush(stdout);                                              \
            abort();                                                     \
        }                                                                \
    } while (0)

#define ASSERT_FALSE(...) ASSERT_FALSE_IMPL(__VA_ARGS__, "", )
#define ASSERT_FALSE_IMPL(result, message, ...)                          \
    do {                                                                 \
        if ((result)) {                                                  \
            TS_FAIL_HEADER(__FILE__, __LINE__, __FUNCTION__, (message)); \
            printf("should be false but got true\n");                    \
            fflush(stdout);                                              \
            abort();                                                     \
        }                                                                \
    } while (0)

void print_success(const char* test_name) {
    double ms_cpu = (double)(clock() - s_cpu_start) * 1000.0 / CLOCKS_PER_SEC;
    double ms_wall;
    struct timespec wall_end;
    clock_gettime(CLOCK_MONOTONIC, &wall_end);
    ms_wall = (double)(wall_end.tv_sec - s_wall_start.tv_sec) * 1000.0 + (double)(wall_end.tv_nsec - s_wall_start.tv_nsec) / 1e6;
    s_tests_run++;
    printf(AN_GREEN "[%ld] '%s' passed." AN_RESET AN_DIM " (cpu: %.3f ms, wall: %.3f ms)\n" AN_RESET, s_tests_run, test_name, ms_cpu, ms_wall);
    fflush(stdout);
}

// TODO: add struct return that contain file and line of failure and
// reason(instead of assert)
#define RUN(fn)                                        \
    do {                                               \
        s_cpu_start = clock();                         \
        clock_gettime(CLOCK_MONOTONIC, &s_wall_start); \
        fn();                                          \
        print_success(#fn);                            \
    } while (0)

#define START_SUITE                              \
    clock_t suite_start = clock();               \
    struct timespec wall_start;                  \
    clock_gettime(CLOCK_MONOTONIC, &wall_start); \
    printf(AN_BOLD AN_CYAN "Running " __FILE__ " tests..." AN_RESET "\n")

#define END_SUITE                                                                                                                               \
    do {                                                                                                                                        \
        double _total_ms_cpu = (double)(clock() - suite_start) * 1000.0 / CLOCKS_PER_SEC;                                                       \
        struct timespec wall_end;                                                                                                               \
        clock_gettime(CLOCK_MONOTONIC, &wall_end);                                                                                              \
        double _total_ms_wall = (double)(wall_end.tv_sec - wall_start.tv_sec) * 1000.0 + (double)(wall_end.tv_nsec - wall_start.tv_nsec) / 1e6; \
        printf(AN_GREEN "\nAll %ld " __FILE__ " tests passed!" AN_RESET AN_DIM " (cpu: %.3f ms, wall: %.3f ms)\n" AN_RESET, s_tests_run,        \
               _total_ms_cpu, _total_ms_wall);                                                                                                  \
    } while (0)

#endif  // TEST_SUITE_H
