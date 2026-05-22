/**
 * @file bench_generic_ring_buffer.c
 * @brief Generic Ring Buffer performance benchmarks
 */

#include "bench_framework.h"
#include <generic_ring_buffer.h>
#include <stdio.h>
#include <stdlib.h>

static void bench_generic_ring_buffer_push_single_int(void) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(1024, sizeof(int));
    if (!rb) {
        printf("Failed to create generic ring buffer\n");
        return;
    }

    const size_t num_ops = 1000000;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        int val = (int)i;
        fc_generic_ring_buffer_push(rb, &val);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "generic_ring_buffer_push_single_int",
        .data_size = num_ops * sizeof(int),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(int)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_generic_ring_buffer_destroy(rb);
}

static void bench_generic_ring_buffer_push_batch_int(void) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(1024, sizeof(int));
    if (!rb) {
        printf("Failed to create generic ring buffer\n");
        return;
    }

    const size_t batch_size = 100;
    const size_t num_batches = 10000;
    int* values = (int*)malloc(batch_size * sizeof(int));
    for (size_t i = 0; i < batch_size; i++) {
        values[i] = (int)i;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_batches; i++) {
        fc_generic_ring_buffer_push_batch(rb, values, batch_size);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    size_t total_ops = num_batches * batch_size;
    fc_bench_result_t result = {
        .name = "generic_ring_buffer_push_batch_int",
        .data_size = total_ops * sizeof(int),
        .elapsed_ms = elapsed_ms,
        .iterations = total_ops,
        .ops_per_sec = (double)total_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)total_ops * sizeof(int)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)total_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    free(values);
    fc_generic_ring_buffer_destroy(rb);
}

static void bench_generic_ring_buffer_pop_single_int(void) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(1024, sizeof(int));
    if (!rb) {
        printf("Failed to create generic ring buffer\n");
        return;
    }

    const size_t num_ops = 1000000;

    for (size_t i = 0; i < num_ops; i++) {
        int val = (int)i;
        fc_generic_ring_buffer_push(rb, &val);
    }

    fc_bench_time_t start = fc_bench_time_now();

    int out;
    for (size_t i = 0; i < num_ops; i++) {
        fc_generic_ring_buffer_pop(rb, &out);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "generic_ring_buffer_pop_single_int",
        .data_size = num_ops * sizeof(int),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(int)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_generic_ring_buffer_destroy(rb);
}

void bench_generic_ring_buffer_run(void) {
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("  Generic Ring Buffer Benchmarks\n");
    printf("------------------------------------------------------------\n");

    bench_generic_ring_buffer_push_single_int();
    bench_generic_ring_buffer_push_batch_int();
    bench_generic_ring_buffer_pop_single_int();
}
