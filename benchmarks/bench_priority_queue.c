/**
 * @file bench_priority_queue.c
 * @brief Priority Queue performance benchmarks
 */

#include "bench_framework.h"
#include <priority_queue.h>
#include <stdio.h>
#include <stdlib.h>

static void bench_priority_queue_insert(void) {
    const size_t capacity = 10000;
    const size_t num_ops = 10000;

    fc_priority_queue_t* pq = fc_priority_queue_create(capacity);
    if (!pq) {
        printf("Failed to create priority queue\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        double priority = (double)i;
        fc_priority_queue_insert(pq, priority, (void*)i);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "priority_queue_insert",
        .data_size = num_ops * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(fc_pq_element_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_priority_queue_destroy(pq);
}

static void bench_priority_queue_pop(void) {
    const size_t capacity = 10000;
    const size_t num_ops = 10000;

    fc_priority_queue_t* pq = fc_priority_queue_create(capacity);
    if (!pq) {
        printf("Failed to create priority queue\n");
        return;
    }

    for (size_t i = 0; i < num_ops; i++) {
        fc_priority_queue_insert(pq, (double)i, (void*)i);
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        double priority;
        void* data;
        fc_priority_queue_pop(pq, &priority, &data);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "priority_queue_pop",
        .data_size = num_ops * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_ops * sizeof(fc_pq_element_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_priority_queue_destroy(pq);
}

static void bench_priority_queue_peek(void) {
    const size_t capacity = 1000;
    const size_t num_ops = 1000000;

    fc_priority_queue_t* pq = fc_priority_queue_create(capacity);
    if (!pq) {
        printf("Failed to create priority queue\n");
        return;
    }

    for (size_t i = 0; i < capacity; i++) {
        fc_priority_queue_insert(pq, (double)i, (void*)i);
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_ops; i++) {
        double priority;
        void* data;
        fc_priority_queue_peek(pq, &priority, &data);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "priority_queue_peek",
        .data_size = num_ops * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_ops,
        .ops_per_sec = (double)num_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = 0.0,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_priority_queue_destroy(pq);
}

static void bench_priority_queue_heapify(void) {
    const size_t num_elements = 10000;
    const size_t num_iterations = 100;

    fc_pq_element_t* elements = (fc_pq_element_t*)malloc(num_elements * sizeof(fc_pq_element_t));
    if (!elements) {
        printf("Failed to allocate elements\n");
        return;
    }

    for (size_t i = 0; i < num_elements; i++) {
        elements[i].priority = (double)(num_elements - i);
        elements[i].data = (void*)i;
    }

    fc_priority_queue_t* pq = fc_priority_queue_create(num_elements);
    if (!pq) {
        printf("Failed to create priority queue\n");
        free(elements);
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_iterations; i++) {
        fc_priority_queue_heapify(pq, elements, num_elements);
        fc_priority_queue_clear(pq);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "priority_queue_heapify",
        .data_size = num_iterations * num_elements * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_iterations,
        .ops_per_sec = (double)(num_iterations * num_elements) / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)(num_iterations * num_elements) * sizeof(fc_pq_element_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_iterations,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    free(elements);
    fc_priority_queue_destroy(pq);
}

static void bench_priority_queue_mixed_operations(void) {
    const size_t capacity = 1000;
    const size_t num_cycles = 10000;

    fc_priority_queue_t* pq = fc_priority_queue_create(capacity);
    if (!pq) {
        printf("Failed to create priority queue\n");
        return;
    }

    for (size_t i = 0; i < capacity / 2; i++) {
        fc_priority_queue_insert(pq, (double)i, (void*)i);
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_cycles; i++) {
        fc_priority_queue_insert(pq, (double)i, (void*)i);

        double priority;
        void* data;
        fc_priority_queue_peek(pq, &priority, &data);

        fc_priority_queue_pop(pq, &priority, &data);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    size_t total_ops = num_cycles * 3;
    fc_bench_result_t result = {
        .name = "priority_queue_mixed_ops",
        .data_size = total_ops * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = total_ops,
        .ops_per_sec = (double)total_ops / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)total_ops * sizeof(fc_pq_element_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)total_ops,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_priority_queue_destroy(pq);
}

static void bench_priority_queue_event_scheduling(void) {
    const size_t num_subscribers = 100;
    const size_t num_events = 10000;

    fc_priority_queue_t* pq = fc_priority_queue_create(num_subscribers * 10);
    if (!pq) {
        printf("Failed to create priority queue\n");
        return;
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < num_events; i++) {
        double timestamp = (double)i + ((double)(i % num_subscribers)) / 100.0;
        fc_priority_queue_insert(pq, timestamp, (void*)i);

        if (i % 10 == 0 && !fc_priority_queue_is_empty(pq)) {
            double priority;
            void* data;
            fc_priority_queue_pop(pq, &priority, &data);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "priority_queue_event_scheduling",
        .data_size = num_events * sizeof(fc_pq_element_t),
        .elapsed_ms = elapsed_ms,
        .iterations = num_events,
        .ops_per_sec = (double)num_events / (elapsed_ms / 1000.0),
        .throughput_gb_s = ((double)num_events * sizeof(fc_pq_element_t)) / (elapsed_ms / 1000.0) / 1e9,
        .gflops = 0.0,
        .mean_ns = (elapsed_ms * 1e6) / (double)num_events,
        .stddev_ns = 0.0,
        .min_ns = 0.0,
        .max_ns = 0.0,
        .bytes_per_op = 0,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_priority_queue_destroy(pq);
}

void bench_priority_queue_register(void) {
    bench_priority_queue_insert();
    bench_priority_queue_pop();
    bench_priority_queue_peek();
    bench_priority_queue_heapify();
    bench_priority_queue_mixed_operations();
    bench_priority_queue_event_scheduling();
}

