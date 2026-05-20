/**
 * @file bench_arena.c
 * @brief Performance benchmarks for arena allocator
 */

#include "arena.h"
#include "bench_framework.h"
#include <stdlib.h>
#include <string.h>

#define SMALL_SIZE 32
#define MEDIUM_SIZE 256
#define LARGE_SIZE 4096

static void bench_arena_alloc_small(void) {
    const size_t iterations = 1000000;
    fc_arena_t* arena = fc_arena_create(iterations * SMALL_SIZE * 2);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_arena_alloc(arena, SMALL_SIZE);
        (void)ptr;
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_alloc_32B",
        .data_size = iterations * SMALL_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = SMALL_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_malloc_small(void) {
    const size_t iterations = 1000000;
    void** ptrs = malloc(sizeof(void*) * iterations);
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        ptrs[i] = malloc(SMALL_SIZE);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "malloc_32B",
        .data_size = iterations * SMALL_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = SMALL_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < iterations; i++) {
        free(ptrs[i]);
    }
    free(ptrs);
}

static void bench_arena_alloc_medium(void) {
    const size_t iterations = 1000000;
    fc_arena_t* arena = fc_arena_create(iterations * MEDIUM_SIZE * 2);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_arena_alloc(arena, MEDIUM_SIZE);
        (void)ptr;
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_alloc_256B",
        .data_size = iterations * MEDIUM_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = MEDIUM_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_malloc_medium(void) {
    const size_t iterations = 1000000;
    void** ptrs = malloc(sizeof(void*) * iterations);
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        ptrs[i] = malloc(MEDIUM_SIZE);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "malloc_256B",
        .data_size = iterations * MEDIUM_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = MEDIUM_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < iterations; i++) {
        free(ptrs[i]);
    }
    free(ptrs);
}

static void bench_arena_alloc_large(void) {
    const size_t iterations = 10000;
    fc_arena_t* arena = fc_arena_create(iterations * LARGE_SIZE * 2);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_arena_alloc(arena, LARGE_SIZE);
        (void)ptr;
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_alloc_4096B",
        .data_size = iterations * LARGE_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = LARGE_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_malloc_large(void) {
    const size_t iterations = 10000;
    void** ptrs = malloc(sizeof(void*) * iterations);
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        ptrs[i] = malloc(LARGE_SIZE);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "malloc_4096B",
        .data_size = iterations * LARGE_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = LARGE_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < iterations; i++) {
        free(ptrs[i]);
    }
    free(ptrs);
}

static void bench_arena_alloc_aligned(void) {
    const size_t iterations = 1000000;
    fc_arena_t* arena = fc_arena_create(iterations * SMALL_SIZE * 2);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_arena_alloc_aligned(arena, SMALL_SIZE, 64);
        (void)ptr;
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_alloc_aligned_64B",
        .data_size = iterations * SMALL_SIZE,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = SMALL_SIZE,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_arena_reset(void) {
    const size_t iterations = 100000;
    fc_arena_t* arena = fc_arena_create(1024 * 1024);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_arena_alloc(arena, 256);
        fc_arena_alloc(arena, 512);
        fc_arena_alloc(arena, 128);
        fc_arena_reset(arena);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_reset",
        .data_size = iterations * (256 + 512 + 128),
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = 256 + 512 + 128,
        .allocs_per_op = 3
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_arena_mixed_sizes(void) {
    const size_t iterations = 100000;
    fc_arena_t* arena = fc_arena_create(iterations * 1024);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_arena_alloc(arena, 16);
        fc_arena_alloc(arena, 64);
        fc_arena_alloc(arena, 128);
        fc_arena_alloc(arena, 256);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_mixed_sizes",
        .data_size = iterations * (16 + 64 + 128 + 256),
        .elapsed_ms = elapsed_ms,
        .iterations = iterations * 4,
        .ops_per_sec = ((iterations * 4) / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / (iterations * 4),
        .bytes_per_op = (16 + 64 + 128 + 256) / 4,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

static void bench_malloc_mixed_sizes(void) {
    const size_t iterations = 100000;
    void** ptrs = malloc(sizeof(void*) * iterations * 4);
    if (!ptrs) {
        fprintf(stderr, "Failed to allocate pointer array\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        ptrs[i * 4 + 0] = malloc(16);
        ptrs[i * 4 + 1] = malloc(64);
        ptrs[i * 4 + 2] = malloc(128);
        ptrs[i * 4 + 3] = malloc(256);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "malloc_mixed_sizes",
        .data_size = iterations * (16 + 64 + 128 + 256),
        .elapsed_ms = elapsed_ms,
        .iterations = iterations * 4,
        .ops_per_sec = ((iterations * 4) / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / (iterations * 4),
        .bytes_per_op = (16 + 64 + 128 + 256) / 4,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < iterations * 4; i++) {
        free(ptrs[i]);
    }
    free(ptrs);
}

static void bench_arena_batch_alloc_reset(void) {
    const size_t outer_iterations = 1000;
    const size_t inner_iterations = 100;
    fc_arena_t* arena = fc_arena_create(1024 * 1024);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < outer_iterations; i++) {
        for (size_t j = 0; j < inner_iterations; j++) {
            void* ptr = fc_arena_alloc(arena, 256);
            (void)ptr;
        }
        fc_arena_reset(arena);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "arena_batch_alloc_reset",
        .data_size = outer_iterations * inner_iterations * 256,
        .elapsed_ms = elapsed_ms,
        .iterations = outer_iterations * inner_iterations,
        .ops_per_sec = ((outer_iterations * inner_iterations) / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / (outer_iterations * inner_iterations),
        .bytes_per_op = 256,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_arena_destroy(arena);
}

void bench_arena_run(void) {
    printf("\n=== Arena Allocator Benchmarks ===\n\n");

    printf("--- Small Allocations (32 bytes) ---\n");
    bench_arena_alloc_small();
    bench_malloc_small();

    printf("\n--- Medium Allocations (256 bytes) ---\n");
    bench_arena_alloc_medium();
    bench_malloc_medium();

    printf("\n--- Large Allocations (4096 bytes) ---\n");
    bench_arena_alloc_large();
    bench_malloc_large();

    printf("\n--- Aligned Allocations ---\n");
    bench_arena_alloc_aligned();

    printf("\n--- Mixed Size Allocations ---\n");
    bench_arena_mixed_sizes();
    bench_malloc_mixed_sizes();

    printf("\n--- Arena-Specific Operations ---\n");
    bench_arena_reset();
    bench_arena_batch_alloc_reset();
}
