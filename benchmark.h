#include <time.h>

double elapsed_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) * 1e-6;
    // double _total_ms_cpu = (double)(clock() - suite_start) * 1000.0 / CLOCKS_PER_SEC;
}

typedef void (*bm_fn)(void* userdata);
typedef void* (*bm_create)(void);

double bm_bench(bm_create create, bm_fn func, bm_fn destroy, size_t runs) {
    {
        void* p                  = create();
        const size_t heatup_runs = 10;
        for (size_t i = 0; i < heatup_runs; i++) {
            func(p);
        }
        destroy(p);
    }

    void* s = create();
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (size_t i = 0; i < runs; i++) {
        func(s);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    destroy(s);
    return elapsed_ms(t0, t1);
}
