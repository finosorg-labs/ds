/**
 * @file bench_roaring_bitmap.c
 * @brief Roaring Bitmap performance benchmarks
 */

#include "bench_framework.h"
#include <roaring_bitmap.h>
#include <mem_aligned.h>
#include <stdio.h>
#include <stdlib.h>

static void bench_roaring_add_single(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    fc_bench_time_t start = fc_bench_time_now();

    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_add_single",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_add_batch(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    uint32_t* values = (uint32_t*)fc_aligned_alloc(num_items * sizeof(uint32_t), 64);
    for (size_t i = 0; i < num_items; i++) {
        values[i] = (uint32_t)i;
    }

    fc_bench_time_t start = fc_bench_time_now();
    fc_roaring_add_batch(bitmap, values, num_items);
    fc_bench_time_t end = fc_bench_time_now();

    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_add_batch",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_items * sizeof(uint32_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_aligned_free(values);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_add_range(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const uint32_t range_size = 100000;
    fc_bench_time_t start = fc_bench_time_now();
    fc_roaring_add_range(bitmap, 0, range_size);
    fc_bench_time_t end = fc_bench_time_now();

    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_add_range",
        .data_size = range_size * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = range_size,
        .ops_per_sec = (double)range_size / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)range_size * sizeof(uint32_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)range_size,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_contains_single(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i);
    }

    bool result;
    fc_bench_time_t start = fc_bench_time_now();

    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_contains(bitmap, i, &result);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t bench_result = {
        .name = "roaring_contains_single",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&bench_result);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_contains_batch(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    uint32_t* values = (uint32_t*)fc_aligned_alloc(num_items * sizeof(uint32_t), 64);
    bool* results = (bool*)fc_aligned_alloc(num_items * sizeof(bool), 64);

    for (size_t i = 0; i < num_items; i++) {
        values[i] = (uint32_t)i;
        fc_roaring_add(bitmap, values[i]);
    }

    fc_bench_time_t start = fc_bench_time_now();
    fc_roaring_contains_batch(bitmap, values, num_items, results);
    fc_bench_time_t end = fc_bench_time_now();

    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_contains_batch",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_items * sizeof(uint32_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_aligned_free(values);
    fc_aligned_free(results);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_cardinality(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i);
    }

    const size_t iterations = 100000;
    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        volatile uint64_t card = fc_roaring_cardinality(bitmap);
        (void)card;
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_cardinality",
        .data_size = 0,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (double)iterations / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)iterations,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_to_array(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i);
    }

    uint32_t* output = (uint32_t*)fc_aligned_alloc(num_items * sizeof(uint32_t), 64);

    fc_bench_time_t start = fc_bench_time_now();
    fc_roaring_to_array(bitmap, output);
    fc_bench_time_t end = fc_bench_time_now();

    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_to_array",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_items * sizeof(uint32_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_aligned_free(output);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_sparse_add(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 10000;
    fc_bench_time_t start = fc_bench_time_now();

    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i * 100);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_sparse_add",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);
    fc_roaring_destroy(bitmap);
}

static void bench_roaring_dense_add(void) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    if (!bitmap) {
        printf("Failed to create Roaring Bitmap\n");
        return;
    }

    const size_t num_items = 50000;
    fc_bench_time_t start = fc_bench_time_now();

    for (uint32_t i = 0; i < num_items; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "roaring_dense_add",
        .data_size = num_items * sizeof(uint32_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint32_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);
    fc_roaring_destroy(bitmap);
}

void bench_roaring_bitmap_run(void) {
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("  Roaring Bitmap Benchmarks\n");
    printf("------------------------------------------------------------\n");
    printf("\n");

    bench_roaring_add_single();
    bench_roaring_add_batch();
    bench_roaring_add_range();
    bench_roaring_contains_single();
    bench_roaring_contains_batch();
    bench_roaring_cardinality();
    bench_roaring_to_array();
    bench_roaring_sparse_add();
    bench_roaring_dense_add();

    printf("\n");
}
