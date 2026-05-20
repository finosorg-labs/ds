/**
 * @file bench_ds.c
 * @brief ds module benchmark entry point
 *
 * This file serves as the main benchmark registration point for the ds module.
 * Individual benchmark modules are in separate files:
 */

#include "bench_framework.h"
#include <simd_detect.h>
#include <stdio.h>

/* External benchmark functions from sub-modules */
extern void bench_bloom_filter_run(void);
extern void bench_roaring_bitmap_run(void);
extern void bench_ring_buffer_run(void);
extern void bench_mem_pool_run(void);
extern void bench_arena_run(void);

/* Entry point for ds benchmarks */
void bench_ds_run(void) {
    printf("\n");
    printf("============================================================\n");
    printf("  ds Module Performance Benchmarks\n");
    printf("  SIMD level: %s\n", fc_simd_level_string(fc_detect_simd()));
    printf("============================================================\n");

    /* Run all sub-module benchmarks */
    bench_bloom_filter_run();
    bench_roaring_bitmap_run();
    bench_ring_buffer_run();
    bench_mem_pool_run();
    bench_arena_run();

    printf("\n");
    printf("============================================================\n");
    printf("  ds benchmarks complete\n");
    printf("============================================================\n");
}
