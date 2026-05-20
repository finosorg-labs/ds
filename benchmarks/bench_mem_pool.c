/**
 * @file bench_mem_pool.c
 * @brief Performance benchmarks for memory pool
 */

#include "mem_pool.h"
#include "bench_framework.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static void bench_mem_pool_alloc_free_single(void) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(1024, 10000);
    if (!pool) {
        fprintf(stderr, "Failed to create pool\n");
        return;
    }

    const size_t iterations = 1000000;
    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_ds_mem_pool_alloc(pool);
        if (ptr) {
            fc_ds_mem_pool_free(pool, ptr);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "mem_pool_alloc_free_single",
        .data_size = iterations * 1024,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = 1024,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
    fc_ds_mem_pool_destroy(pool);
}

static void bench_mem_pool_alloc_free_batch(void) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(512, 10000);
    if (!pool) {
        fprintf(stderr, "Failed to create pool\n");
        return;
    }

    const size_t batch_size = 100;
    const size_t iterations = 10000;
    void* ptrs[100];

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        size_t allocated = fc_ds_mem_pool_alloc_batch(pool, batch_size, ptrs);
        if (allocated > 0) {
            fc_ds_mem_pool_free_batch(pool, (void* const*)ptrs, allocated);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "mem_pool_alloc_free_batch_100",
        .data_size = iterations * batch_size * 512,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations * batch_size,
        .ops_per_sec = ((iterations * batch_size) / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / (iterations * batch_size),
        .bytes_per_op = 512,
        .allocs_per_op = batch_size
    };

    fc_bench_result_print(&result);
    fc_ds_mem_pool_destroy(pool);
}

static void bench_mem_pool_vs_malloc_single(void) {
    const size_t iterations = 100000;
    const size_t block_size = 1024;

    fc_bench_time_t start_malloc = fc_bench_time_now();
    for (size_t i = 0; i < iterations; i++) {
        void* ptr = malloc(block_size);
        if (ptr) {
            free(ptr);
        }
    }
    fc_bench_time_t end_malloc = fc_bench_time_now();
    double malloc_ms = fc_bench_time_elapsed_ms(&start_malloc, &end_malloc);

    fc_mem_pool_t* pool = fc_ds_mem_pool_create(block_size, 10000);
    if (!pool) {
        fprintf(stderr, "Failed to create pool\n");
        return;
    }

    fc_bench_time_t start_pool = fc_bench_time_now();
    for (size_t i = 0; i < iterations; i++) {
        void* ptr = fc_ds_mem_pool_alloc(pool);
        if (ptr) {
            fc_ds_mem_pool_free(pool, ptr);
        }
    }
    fc_bench_time_t end_pool = fc_bench_time_now();
    double pool_ms = fc_bench_time_elapsed_ms(&start_pool, &end_pool);

    printf("\n=== Memory Pool vs malloc Comparison ===\n");
    printf("Iterations: %zu\n", iterations);
    printf("Block size: %zu bytes\n", block_size);
    printf("malloc:     %.3f ms (%.0f ns/op, %.2f M ops/sec)\n",
           malloc_ms,
           (malloc_ms * 1000000.0) / iterations,
           (iterations / malloc_ms) / 1000.0);
    printf("mem_pool:   %.3f ms (%.0f ns/op, %.2f M ops/sec)\n",
           pool_ms,
           (pool_ms * 1000000.0) / iterations,
           (iterations / pool_ms) / 1000.0);
    printf("Speedup:    %.2fx\n", malloc_ms / pool_ms);
    printf("\n");

    fc_ds_mem_pool_destroy(pool);
}

static void bench_mem_pool_different_sizes(void) {
    const size_t sizes[] = {64, 128, 256, 512, 1024, 2048, 4096};
    const size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const size_t iterations = 100000;

    printf("\n=== Memory Pool Performance by Block Size ===\n");
    printf("%-10s %-15s %-15s %-15s\n", "Size", "Time (ms)", "ns/op", "M ops/sec");
    printf("--------------------------------------------------------\n");

    for (size_t i = 0; i < num_sizes; i++) {
        size_t block_size = sizes[i];
        fc_mem_pool_t* pool = fc_ds_mem_pool_create(block_size, 10000);
        if (!pool) {
            continue;
        }

        fc_bench_time_t start = fc_bench_time_now();
        for (size_t j = 0; j < iterations; j++) {
            void* ptr = fc_ds_mem_pool_alloc(pool);
            if (ptr) {
                fc_ds_mem_pool_free(pool, ptr);
            }
        }
        fc_bench_time_t end = fc_bench_time_now();
        double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

        printf("%-10zu %-15.3f %-15.0f %-15.2f\n",
               block_size,
               elapsed_ms,
               (elapsed_ms * 1000000.0) / iterations,
               (iterations / elapsed_ms) / 1000.0);

        fc_ds_mem_pool_destroy(pool);
    }
    printf("\n");
}

typedef struct {
    fc_mem_pool_t* pool;
    size_t iterations;
    double elapsed_ms;
} thread_bench_args_t;

static void* thread_bench_worker(void* arg) {
    thread_bench_args_t* args = (thread_bench_args_t*)arg;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < args->iterations; i++) {
        void* ptr = fc_ds_mem_pool_alloc(args->pool);
        if (ptr) {
            fc_ds_mem_pool_free(args->pool, ptr);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    args->elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    return NULL;
}

static void bench_mem_pool_multithreaded(void) {
    const int thread_counts[] = {1, 2, 4, 8};
    const size_t num_configs = sizeof(thread_counts) / sizeof(thread_counts[0]);
    const size_t iterations_per_thread = 100000;

    printf("\n=== Memory Pool Multi-threaded Performance ===\n");
    printf("%-10s %-15s %-15s %-15s %-15s\n",
           "Threads", "Total ops", "Time (ms)", "M ops/sec", "Speedup");
    printf("------------------------------------------------------------------------\n");

    double baseline_time = 0.0;

    for (size_t i = 0; i < num_configs; i++) {
        int num_threads = thread_counts[i];
        fc_mem_pool_t* pool = fc_ds_mem_pool_create(1024, 100000);
        if (!pool) {
            continue;
        }

        pthread_t threads[8];
        thread_bench_args_t args[8];

        fc_bench_time_t start = fc_bench_time_now();

        for (int j = 0; j < num_threads; j++) {
            args[j].pool = pool;
            args[j].iterations = iterations_per_thread;
            args[j].elapsed_ms = 0.0;
            pthread_create(&threads[j], NULL, thread_bench_worker, &args[j]);
        }

        for (int j = 0; j < num_threads; j++) {
            pthread_join(threads[j], NULL);
        }

        fc_bench_time_t end = fc_bench_time_now();
        double total_elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);
        size_t total_ops = iterations_per_thread * num_threads;

        if (i == 0) {
            baseline_time = total_elapsed_ms;
        }

        printf("%-10d %-15zu %-15.3f %-15.2f %-15.2fx\n",
               num_threads,
               total_ops,
               total_elapsed_ms,
               (total_ops / total_elapsed_ms) / 1000.0,
               baseline_time / total_elapsed_ms);

        fc_ds_mem_pool_destroy(pool);
    }
    printf("\n");
}

static void bench_mem_pool_high_contention(void) {
    const size_t pool_size = 100;
    const size_t iterations = 50000;
    const int num_threads = 4;

    fc_mem_pool_t* pool = fc_ds_mem_pool_create(256, pool_size);
    if (!pool) {
        fprintf(stderr, "Failed to create pool\n");
        return;
    }

    pthread_t threads[4];
    thread_bench_args_t args[4];

    fc_bench_time_t start = fc_bench_time_now();

    for (int i = 0; i < num_threads; i++) {
        args[i].pool = pool;
        args[i].iterations = iterations;
        args[i].elapsed_ms = 0.0;
        pthread_create(&threads[i], NULL, thread_bench_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double total_elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    printf("\n=== High Contention Scenario ===\n");
    printf("Pool size: %zu blocks\n", pool_size);
    printf("Threads: %d\n", num_threads);
    printf("Iterations per thread: %zu\n", iterations);
    printf("Total time: %.3f ms\n", total_elapsed_ms);
    printf("Throughput: %.2f M ops/sec\n",
           ((iterations * num_threads) / total_elapsed_ms) / 1000.0);
    printf("\n");

    fc_ds_mem_pool_destroy(pool);
}

static void bench_mem_pool_realistic_workload(void) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(512, 5000);
    if (!pool) {
        fprintf(stderr, "Failed to create pool\n");
        return;
    }

    const size_t iterations = 10000;
    void* active_ptrs[100];
    size_t active_count = 0;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        if (active_count < 100 && (rand() % 100) < 60) {
            void* ptr = fc_ds_mem_pool_alloc(pool);
            if (ptr) {
                active_ptrs[active_count++] = ptr;
            }
        }

        if (active_count > 0 && (rand() % 100) < 40) {
            size_t idx = rand() % active_count;
            fc_ds_mem_pool_free(pool, active_ptrs[idx]);
            active_ptrs[idx] = active_ptrs[--active_count];
        }
    }

    while (active_count > 0) {
        fc_ds_mem_pool_free(pool, active_ptrs[--active_count]);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_mem_pool_stats_t stats;
    fc_ds_mem_pool_get_stats(pool, &stats);

    printf("\n=== Realistic Workload (Mixed Alloc/Free) ===\n");
    printf("Iterations: %zu\n", iterations);
    printf("Total time: %.3f ms\n", elapsed_ms);
    printf("Total allocations: %zu\n", stats.alloc_count);
    printf("Total frees: %zu\n", stats.free_count);
    printf("Peak usage: %zu blocks\n", stats.peak_usage);
    printf("Avg time per iteration: %.0f ns\n", (elapsed_ms * 1000000.0) / iterations);
    printf("\n");

    fc_ds_mem_pool_destroy(pool);
}

void bench_mem_pool_run(void) {
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("  Memory Pool Benchmarks\n");
    printf("------------------------------------------------------------\n");
    printf("Timer resolution: %lu ns\n\n", fc_bench_get_timer_resolution_ns());

    bench_mem_pool_alloc_free_single();
    bench_mem_pool_alloc_free_batch();
    bench_mem_pool_vs_malloc_single();
    bench_mem_pool_different_sizes();
    bench_mem_pool_multithreaded();
    bench_mem_pool_high_contention();
    bench_mem_pool_realistic_workload();

    printf("------------------------------------------------------------\n");
}
