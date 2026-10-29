#include <math.h>
#include <stdio.h>

#include "benchmark.h"
#define STRING_BUILDER_LL_IMPLEMENTATION
#include "string/string_builder_ll.h"

// bn_fn
void bench_append(void* userdata) {
    sb_append(userdata, "Hello");
    sb_append(userdata, " world");
    sb_append(userdata, "!");
    sb_append(userdata, " The quick brown fox");
    sb_append(userdata, " jumps over");
    sb_append(userdata, " the lazy dog.");
    sb_append(userdata, " foo");
    sb_append(userdata, " bar");
    sb_append(userdata, " baz");
    sb_append(userdata, " 1234567890");
    sb_append(userdata, " abcdefghij");
    sb_append(userdata, " ABCDEFGHIJ");
}

// bn_fn
void destroy(void* userdata) { sb_destroy(userdata); }

// bn_create
void* create(void) { return sb_create(); }
void* create_dump(void) {
    string_builder* sb = sb_create();
    if (sb == NULL) {
        fprintf(stderr, "Failed to create string builder!\n");
        return NULL;
    }
    for (int i = 0; i < 1000; i++) {
        sb_append(sb, "Hello");
        sb_append(sb, " world");
        sb_append(sb, "!");
        sb_append(sb, " The quick brown fox");
        sb_append(sb, " jumps over");
        sb_append(sb, " the lazy dog.");
        sb_append(sb, " foo");
        sb_append(sb, " bar");
        sb_append(sb, " baz");
        sb_append(sb, " 1234567890");
        sb_append(sb, " abcdefghij");
        sb_append(sb, " ABCDEFGHIJ");
    }
    return sb;
}
void dump(void* userdata) {
    const char* str = sb_get_string(userdata);
    if (str == NULL) {
        fprintf(stderr, "Failed to get string from string builder!\n");
        return;
    }
}

#define BENCHMARK(create, func, destroy, runs)                                        \
    do {                                                                              \
        struct report r = bm_run(create, func, destroy, runs);                        \
        printf(                                                                       \
            "%-20s: max=%10.3f ms, min=%10.3f ms, median=%10.3f ms, mean=%10.3f ms, " \
            "stddev=%10.3f ms\n",                                                     \
            #func, r.max, r.min, r.median, r.mean, r.stddev);                         \
    } while (0)

struct report {
    double max;
    double min;
    double median;
    double mean;
    double stddev;
};

int cmp_double(const void* a, const void* b) {
    double x = *(double*)a, y = *(double*)b;
    return (x > y) - (x < y);
}

struct report bm_run(bm_create create, bm_fn func, bm_fn destroy, size_t runs) {
    const size_t warmup_runs = 100;
    for (size_t i = 0; i < warmup_runs; i++) {
        void* p = create();
        func(p);
        destroy(p);
    }
    const size_t benchmark_runs = 100;
    double runs_array[benchmark_runs];
    for (size_t i = 0; i < benchmark_runs; i++) {
        runs_array[i] = bm_bench(create, func, destroy, runs);
    }
    qsort(runs_array, benchmark_runs, sizeof(double), cmp_double);
    struct report r;
    r.max      = runs_array[benchmark_runs - 1];
    r.min      = runs_array[0];
    r.median   = runs_array[benchmark_runs / 2];
    double sum = 0.0;
    for (size_t i = 0; i < benchmark_runs; i++) {
        sum += runs_array[i];
    }
    r.mean             = sum / (double)benchmark_runs;
    double sq_diff_sum = 0.0;
    for (size_t i = 0; i < benchmark_runs; i++) {
        double diff = runs_array[i] - r.mean;
        sq_diff_sum += diff * diff;
    }
    r.stddev = sqrt(sq_diff_sum / (double)benchmark_runs);
    return r;
}

#define WARMUP                                             \
    for (int i = 0; i < 10000000; i++) {                   \
        volatile int x = i * i; /* prevent optimization */ \
        (void)x;                                           \
    }

int main(void) {
    WARMUP;
    BENCHMARK(create, bench_append, destroy, 1000);
    BENCHMARK(create_dump, dump, destroy, 1000);
    BENCHMARK(create, bench_append, destroy, 1000);
    BENCHMARK(create_dump, dump, destroy, 1000);
    BENCHMARK(create, bench_append, destroy, 1000);
    BENCHMARK(create_dump, dump, destroy, 1000);
    BENCHMARK(create, bench_append, destroy, 1000);
    BENCHMARK(create_dump, dump, destroy, 1000);
    return 0;
}