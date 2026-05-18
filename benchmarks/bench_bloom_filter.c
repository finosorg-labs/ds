/**
 * @file bench_bloom_filter.c
 * @brief Bloom Filter performance benchmarks
 */

#include "bench_framework.h"
#include <bloom_filter.h>
#include <mem_aligned.h>
#include <stdio.h>
#include <string.h>

static void bench_bloom_add_single(void) {
    fc_bloom_config_t config = {
        .expected_elements = 100000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    if (!filter) {
        printf("Failed to create Bloom filter\n");
        return;
    }

    const size_t num_items = 10000;
    uint64_t* items = (uint64_t*)fc_aligned_alloc(num_items * sizeof(uint64_t), 64);
    for (size_t i = 0; i < num_items; i++) {
        items[i] = i;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_items; i++) {
        fc_bloom_add(filter, &items[i], sizeof(uint64_t));
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "bloom_add_single",
        .data_size = num_items * sizeof(uint64_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint64_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_aligned_free(items);
    fc_bloom_destroy(filter);
}

static void bench_bloom_add_batch(void) {
    fc_bloom_config_t config = {
        .expected_elements = 100000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    if (!filter) {
        printf("Failed to create Bloom filter\n");
        return;
    }

    const size_t num_items = 10000;
    uint64_t* items = (uint64_t*)fc_aligned_alloc(num_items * sizeof(uint64_t), 64);
    const void** item_ptrs = (const void**)fc_aligned_alloc(num_items * sizeof(void*), 64);
    size_t* lengths = (size_t*)fc_aligned_alloc(num_items * sizeof(size_t), 64);

    for (size_t i = 0; i < num_items; i++) {
        items[i] = i;
        item_ptrs[i] = &items[i];
        lengths[i] = sizeof(uint64_t);
    }

    fc_bench_time_t start = fc_bench_time_now();

    fc_bloom_add_batch(filter, item_ptrs, lengths, num_items);

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "bloom_add_batch",
        .data_size = num_items * sizeof(uint64_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint64_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_aligned_free(lengths);
    fc_aligned_free(item_ptrs);
    fc_aligned_free(items);
    fc_bloom_destroy(filter);
}

static void bench_bloom_contains_single(void) {
    fc_bloom_config_t config = {
        .expected_elements = 100000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    if (!filter) {
        printf("Failed to create Bloom filter\n");
        return;
    }

    const size_t num_items = 10000;
    uint64_t* items = (uint64_t*)fc_aligned_alloc(num_items * sizeof(uint64_t), 64);
    for (size_t i = 0; i < num_items; i++) {
        items[i] = i;
        fc_bloom_add(filter, &items[i], sizeof(uint64_t));
    }

    fc_bench_time_t start = fc_bench_time_now();

    bool result;
    for (size_t i = 0; i < num_items; i++) {
        fc_bloom_contains(filter, &items[i], sizeof(uint64_t), &result);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t bench_result = {
        .name = "bloom_contains_single",
        .data_size = num_items * sizeof(uint64_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint64_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&bench_result);

    fc_aligned_free(items);
    fc_bloom_destroy(filter);
}

static void bench_bloom_contains_batch(void) {
    fc_bloom_config_t config = {
        .expected_elements = 100000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    if (!filter) {
        printf("Failed to create Bloom filter\n");
        return;
    }

    const size_t num_items = 10000;
    uint64_t* items = (uint64_t*)fc_aligned_alloc(num_items * sizeof(uint64_t), 64);
    const void** item_ptrs = (const void**)fc_aligned_alloc(num_items * sizeof(void*), 64);
    size_t* lengths = (size_t*)fc_aligned_alloc(num_items * sizeof(size_t), 64);
    bool* results = (bool*)fc_aligned_alloc(num_items * sizeof(bool), 64);

    for (size_t i = 0; i < num_items; i++) {
        items[i] = i;
        item_ptrs[i] = &items[i];
        lengths[i] = sizeof(uint64_t);
        fc_bloom_add(filter, &items[i], sizeof(uint64_t));
    }

    fc_bench_time_t start = fc_bench_time_now();

    fc_bloom_contains_batch(filter, item_ptrs, lengths, num_items, results);

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t bench_result = {
        .name = "bloom_contains_batch",
        .data_size = num_items * sizeof(uint64_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint64_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&bench_result);

    fc_aligned_free(results);
    fc_aligned_free(lengths);
    fc_aligned_free(item_ptrs);
    fc_aligned_free(items);
    fc_bloom_destroy(filter);
}

static void bench_bloom_mixed_workload(void) {
    fc_bloom_config_t config = {
        .expected_elements = 100000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    if (!filter) {
        printf("Failed to create Bloom filter\n");
        return;
    }

    const size_t num_items = 10000;
    uint64_t* items = (uint64_t*)fc_aligned_alloc(num_items * sizeof(uint64_t), 64);
    for (size_t i = 0; i < num_items; i++) {
        items[i] = i;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_items; i++) {
        fc_bloom_add(filter, &items[i], sizeof(uint64_t));

        if (i > 0 && i % 10 == 0) {
            bool result;
            fc_bloom_contains(filter, &items[i - 1], sizeof(uint64_t), &result);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t bench_result = {
        .name = "bloom_mixed_workload",
        .data_size = num_items * sizeof(uint64_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_items,
        .ops_per_sec = (double)num_items / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_items,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = sizeof(uint64_t),
        .allocs_per_op = 0
    };

    fc_bench_result_print(&bench_result);

    fc_aligned_free(items);
    fc_bloom_destroy(filter);
}

void bench_bloom_filter_run(void) {
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("  Bloom Filter Benchmarks\n");
    printf("------------------------------------------------------------\n");

    bench_bloom_add_single();
    bench_bloom_add_batch();
    bench_bloom_contains_single();
    bench_bloom_contains_batch();
    bench_bloom_mixed_workload();

    printf("------------------------------------------------------------\n");
}
