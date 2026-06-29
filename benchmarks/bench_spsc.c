#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define fc_cpu_pause() __builtin_ia32_pause()
#elif defined(__aarch64__) || defined(__arm__)
    #define fc_cpu_pause() __asm__ __volatile__("yield")
#else
    #define fc_cpu_pause() do {} while(0)
#endif

/**
 * @file bench_spsc.c
 * @brief Benchmarks for SPSC ring buffer
 */

#include "spsc.h"
#include "bench_framework.h"
#include <mem_aligned.h>
#include <pthread.h>
#include <unistd.h>

/* Benchmark data */
typedef struct {
    fc_spsc_t* queue;
    int64_t* data;
    size_t count;
} bench_data_t;

/* Single-threaded push benchmark */
static void bench_push_fn(void* user_data) {
    bench_data_t* data = (bench_data_t*)user_data;
    int64_t elem = 42;
    fc_spsc_push(data->queue, &elem);
}

/* Single-threaded pop benchmark */
static void bench_pop_fn(void* user_data) {
    bench_data_t* data = (bench_data_t*)user_data;
    int64_t result;
    fc_spsc_pop(data->queue, &result);
}

/* Push-pop pair benchmark */
static void bench_push_pop_fn(void* user_data) {
    bench_data_t* data = (bench_data_t*)user_data;
    int64_t elem = 42;
    int64_t result;
    fc_spsc_push(data->queue, &elem);
    fc_spsc_pop(data->queue, &result);
}

/* Bulk push benchmark */
static void bench_bulk_push_fn(void* user_data) {
    bench_data_t* data = (bench_data_t*)user_data;
    fc_spsc_push_bulk(data->queue, data->data, 100);
}

/* Bulk pop benchmark */
static void bench_bulk_pop_fn(void* user_data) {
    bench_data_t* data = (bench_data_t*)user_data;
    fc_spsc_pop_bulk(data->queue, data->data, 100);
}

/* Concurrent benchmark data */
typedef struct {
    fc_spsc_t* producer_queue;
    fc_spsc_t* consumer_queue;
    size_t count;
    volatile int* start_flag;
} concurrent_bench_data_t;

static void* concurrent_producer_thread(void* arg) {
    concurrent_bench_data_t* data = (concurrent_bench_data_t*)arg;

    /* Wait for start signal */
    while (!*data->start_flag) {
        fc_cpu_pause();
    }

    for (size_t i = 0; i < data->count; i++) {
        int64_t elem = i;
        while (fc_spsc_push(data->producer_queue, &elem) != FC_OK) {
            fc_cpu_pause();
        }
    }

    return NULL;
}

static void* concurrent_consumer_thread(void* arg) {
    concurrent_bench_data_t* data = (concurrent_bench_data_t*)arg;
    size_t received = 0;

    /* Wait for start signal */
    while (!*data->start_flag) {
        fc_cpu_pause();
    }

    while (received < data->count) {
        int64_t result;
        if (fc_spsc_pop(data->consumer_queue, &result) == FC_OK) {
            received++;
        } else {
            fc_cpu_pause();
        }
    }

    return NULL;
}

void bench_spsc_push_only(void) {
    fc_spsc_t producer;

    size_t capacity = 1048576;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);

    bench_data_t data = {&producer, NULL, 0};

    fc_bench_config_t config = FC_BENCH_CONFIG_DEFAULT;
    config.name = "spsc_push_only";
    config.data_size = sizeof(int64_t);
    config.min_iterations = 10000;
    config.min_time_ms = 500.0;

    fc_bench_result_t result;
    fc_bench_run(&config, bench_push_fn, &data, &result);
    fc_bench_result_print(&result);

    fc_aligned_free(mem);
}

void bench_spsc_pop_only(void) {
    fc_spsc_t producer, consumer;

    size_t capacity = 1048576;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Prefill */
    int64_t elem = 42;
    for (size_t i = 0; i < capacity; i++) {
        fc_spsc_push(&producer, &elem);
    }

    bench_data_t data = {&consumer, NULL, 0};

    fc_bench_config_t config = FC_BENCH_CONFIG_DEFAULT;
    config.name = "spsc_pop_only";
    config.data_size = sizeof(int64_t);
    config.min_iterations = 10000;
    config.min_time_ms = 500.0;

    fc_bench_result_t result;
    fc_bench_run(&config, bench_pop_fn, &data, &result);
    fc_bench_result_print(&result);

    fc_aligned_free(mem);
}

void bench_spsc_push_pop_pair(void) {
    fc_spsc_t producer;

    size_t capacity = 1024;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);

    bench_data_t data = {&producer, NULL, 0};

    fc_bench_config_t config = FC_BENCH_CONFIG_DEFAULT;
    config.name = "spsc_push_pop_pair";
    config.data_size = sizeof(int64_t) * 2; /* push + pop */
    config.min_iterations = 10000;
    config.min_time_ms = 500.0;

    fc_bench_result_t result;
    fc_bench_run(&config, bench_push_pop_fn, &data, &result);
    fc_bench_result_print(&result);

    fc_aligned_free(mem);
}

void bench_spsc_bulk_push(void) {
    fc_spsc_t producer;

    size_t capacity = 1048576;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);

    /* Prepare batch data */
    int64_t batch[100];
    for (size_t i = 0; i < 100; i++) {
        batch[i] = i;
    }

    bench_data_t data = {&producer, batch, 0};

    fc_bench_config_t config = FC_BENCH_CONFIG_DEFAULT;
    config.name = "spsc_bulk_push_100";
    config.data_size = 100 * sizeof(int64_t);
    config.min_iterations = 1000;
    config.min_time_ms = 500.0;

    fc_bench_result_t result;
    fc_bench_run(&config, bench_bulk_push_fn, &data, &result);
    fc_bench_result_print(&result);

    fc_aligned_free(mem);
}

void bench_spsc_bulk_pop(void) {
    fc_spsc_t producer, consumer;

    size_t capacity = 1048576;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Prefill */
    const size_t batch_size = 100;
    int64_t batch[100];
    for (size_t i = 0; i < batch_size; i++) {
        batch[i] = i;
    }
    for (size_t i = 0; i < capacity / batch_size; i++) {
        fc_spsc_push_bulk(&producer, batch, batch_size);
    }

    bench_data_t data = {&consumer, batch, 0};

    fc_bench_config_t config = FC_BENCH_CONFIG_DEFAULT;
    config.name = "spsc_bulk_pop_100";
    config.data_size = 100 * sizeof(int64_t);
    config.min_iterations = 1000;
    config.min_time_ms = 500.0;

    fc_bench_result_t result;
    fc_bench_run(&config, bench_bulk_pop_fn, &data, &result);
    fc_bench_result_print(&result);

    fc_aligned_free(mem);
}

void bench_spsc_concurrent(void) {
    fc_spsc_t producer, consumer;

    size_t capacity = 65536;
    size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);

    fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    const size_t count = 1000000;
    volatile int start_flag = 0;

    concurrent_bench_data_t prod_data = {&producer, NULL, count, &start_flag};
    concurrent_bench_data_t cons_data = {NULL, &consumer, count, &start_flag};

    pthread_t prod_thread, cons_thread;
    pthread_create(&prod_thread, NULL, concurrent_producer_thread, &prod_data);
    pthread_create(&cons_thread, NULL, concurrent_consumer_thread, &cons_data);

    /* Brief warm-up */
    usleep(10000);

    fc_bench_time_t start = fc_bench_time_now();
    start_flag = 1;  /* Start both threads */

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    fc_bench_time_t end = fc_bench_time_now();

    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);
    double ops_per_sec = fc_bench_ops_per_sec(count, elapsed_ms);
    double throughput = fc_bench_throughput_gb_s(count * sizeof(int64_t), elapsed_ms);

    printf("spsc_concurrent: %.2f ms, %.2f Mops/s, %.2f GB/s\n",
           elapsed_ms, ops_per_sec / 1e6, throughput);

    fc_aligned_free(mem);
}

void bench_spsc_different_capacities(void) {
    const uint32_t capacities[] = {16, 64, 256, 1024, 4096, 16384};
    const size_t num_capacities = sizeof(capacities) / sizeof(capacities[0]);

    printf("\n=== SPSC Ring Buffer - Different Capacities ===\n");
    printf("%-10s %-15s %-15s\n", "Capacity", "Push (ns/op)", "Pop (ns/op)");
    printf("------------------------------------------------\n");

    for (size_t i = 0; i < num_capacities; i++) {
        uint32_t capacity = capacities[i];
        size_t size = fc_spsc_arena_size(capacity, sizeof(int64_t));
        void* mem = fc_aligned_alloc(size, 64);

        fc_spsc_t producer, consumer;
        fc_spsc_init(&producer, mem, size, capacity, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
        fc_spsc_attach(&consumer, mem, size);

        /* Benchmark push */
        bench_data_t push_data = {&producer, NULL, 0};
        fc_bench_config_t push_config = FC_BENCH_CONFIG_DEFAULT;
        push_config.name = "push";
        push_config.quiet = 1;
        push_config.min_iterations = 10000;

        fc_bench_result_t push_result;
        fc_bench_run(&push_config, bench_push_fn, &push_data, &push_result);

        /* Prefill for pop benchmark */
        int64_t elem = 42;
        for (uint32_t j = 0; j < capacity; j++) {
            fc_spsc_push(&producer, &elem);
        }

        /* Benchmark pop */
        bench_data_t pop_data = {&consumer, NULL, 0};
        fc_bench_config_t pop_config = FC_BENCH_CONFIG_DEFAULT;
        pop_config.name = "pop";
        pop_config.quiet = 1;
        pop_config.min_iterations = 10000;

        fc_bench_result_t pop_result;
        fc_bench_run(&pop_config, bench_pop_fn, &pop_data, &pop_result);

        printf("%-10u %-15.2f %-15.2f\n",
               capacity, push_result.mean_ns, pop_result.mean_ns);

        fc_aligned_free(mem);
    }
}

void bench_spsc_run(void) {
    fc_bench_init();

    printf("\n=== SPSC Ring Buffer Benchmarks ===\n\n");

    fc_bench_print_header();

    bench_spsc_push_only();
    bench_spsc_pop_only();
    bench_spsc_push_pop_pair();
    bench_spsc_bulk_push();
    bench_spsc_bulk_pop();
    bench_spsc_concurrent();

    bench_spsc_different_capacities();

    fc_bench_cleanup();

}
