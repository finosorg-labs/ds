/**
 * @file bench_ring_buffer.c
 * @brief Ring Buffer performance benchmarks
 */

#include "bench_framework.h"
#include <ring_buffer.h>
#include <stdio.h>
#include <stdlib.h>

static void bench_ring_buffer_push_single(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(1024);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    const size_t num_ops = 1000000;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        fc_ring_buffer_push(rb, (double)i);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "ring_buffer_push_single",
        .data_size = num_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_ring_buffer_destroy(rb);
}

static void bench_ring_buffer_push_batch(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(1024);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    const size_t batch_size = 100;
    const size_t num_batches = 10000;
    double* values = (double*)malloc(batch_size * sizeof(double));
    for (size_t i = 0; i < batch_size; i++) {
        values[i] = (double)i;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_batches; i++) {
        fc_ring_buffer_push_batch(rb, values, batch_size);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    size_t total_ops = num_batches * batch_size;
    fc_bench_result_t result = {
        .name = "ring_buffer_push_batch",
        .data_size = total_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = total_ops,
        .ops_per_sec = (double)total_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)total_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
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
    fc_ring_buffer_destroy(rb);
}

static void bench_ring_buffer_pop_single(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(1024);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    const size_t num_ops = 1000000;
    for (size_t i = 0; i < 1024; i++) {
        fc_ring_buffer_push(rb, (double)i);
    }

    double val;
    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        fc_ring_buffer_pop(rb, &val);
        if (fc_ring_buffer_is_empty(rb)) {
            fc_ring_buffer_push(rb, (double)i);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "ring_buffer_pop_single",
        .data_size = num_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_ring_buffer_destroy(rb);
}

static void bench_ring_buffer_pop_batch(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(1024);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    const size_t batch_size = 100;
    const size_t num_batches = 10000;

    for (size_t i = 0; i < 1024; i++) {
        fc_ring_buffer_push(rb, (double)i);
    }

    double* values = (double*)malloc(batch_size * sizeof(double));

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_batches; i++) {
        fc_ring_buffer_pop_batch(rb, values, batch_size);
        if (fc_ring_buffer_size(rb) < batch_size) {
            for (size_t j = 0; j < batch_size; j++) {
                fc_ring_buffer_push(rb, (double)(i * batch_size + j));
            }
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    size_t total_ops = num_batches * batch_size;
    fc_bench_result_t result = {
        .name = "ring_buffer_pop_batch",
        .data_size = total_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = total_ops,
        .ops_per_sec = (double)total_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)total_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
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
    fc_ring_buffer_destroy(rb);
}

static void bench_ring_buffer_get_random(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(1024);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    for (size_t i = 0; i < 1024; i++) {
        fc_ring_buffer_push(rb, (double)i);
    }

    const size_t num_ops = 1000000;
    double val;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        size_t index = i % fc_ring_buffer_size(rb);
        fc_ring_buffer_get(rb, index, &val);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "ring_buffer_get_random",
        .data_size = num_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_ring_buffer_destroy(rb);
}

static void bench_ring_buffer_sliding_window(void) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(100);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }

    const size_t num_ops = 1000000;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        fc_ring_buffer_push(rb, (double)i);

        if (i % 10 == 0 && fc_ring_buffer_size(rb) == 100) {
            double sum = 0.0;
            for (size_t j = 0; j < fc_ring_buffer_size(rb); j++) {
                double val;
                fc_ring_buffer_get(rb, j, &val);
                sum += val;
            }
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "ring_buffer_sliding_window",
        .data_size = num_ops * sizeof(double),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(double)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_ring_buffer_destroy(rb);
}

void bench_ring_buffer_run(void) {
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("  Ring Buffer Benchmarks\n");
    printf("------------------------------------------------------------\n");

    bench_ring_buffer_push_single();
    bench_ring_buffer_push_batch();
    bench_ring_buffer_pop_single();
    bench_ring_buffer_pop_batch();
    bench_ring_buffer_get_random();
    bench_ring_buffer_sliding_window();

    printf("------------------------------------------------------------\n");
}
