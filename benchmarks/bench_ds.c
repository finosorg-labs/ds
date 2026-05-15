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

/* Entry point for ds benchmarks */
void bench_ds_run(void) {
    printf("\n");
    printf("============================================================\n");
    printf("  ds Module Performance Benchmarks\n");
    printf("  SIMD level: %s\n", fc_simd_level_string(fc_detect_simd()));
    printf("============================================================\n");

    /* Run all sub-module benchmarks */

    printf("\n");
    printf("============================================================\n");
    printf("  ds benchmarks complete\n");
    printf("============================================================\n");
}
