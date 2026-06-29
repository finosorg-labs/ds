/**
 * @file bench_spsc_shm.c
 * @brief Performance benchmarks for SPSC shared memory operations
 */

#include "spsc_shm.h"
#include "bench_framework.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

static void generate_unique_name(char* buf, size_t len) {
    snprintf(buf, len, "bench_spsc_%ld_%d", (long)time(NULL), rand() % 10000);
}

static void bench_shm_create_4k(void) {
    const size_t iterations = 10000;
    const size_t size = 4096;
    char names[100][256];

    for (size_t i = 0; i < 100; i++) {
        generate_unique_name(names[i], sizeof(names[i]));
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_spsc_shm_t shm;
        fc_spsc_shm_create(&shm, names[i % 100], size);
        fc_spsc_shm_close(&shm);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_create_4KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < 100; i++) {
        fc_spsc_shm_unlink(names[i]);
    }
}

static void bench_shm_create_1m(void) {
    const size_t iterations = 1000;
    const size_t size = 1024 * 1024;
    char names[100][256];

    for (size_t i = 0; i < 100; i++) {
        generate_unique_name(names[i], sizeof(names[i]));
    }

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_spsc_shm_t shm;
        fc_spsc_shm_create(&shm, names[i % 100], size);
        fc_spsc_shm_close(&shm);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_create_1MB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);

    for (size_t i = 0; i < 100; i++) {
        fc_spsc_shm_unlink(names[i]);
    }
}

static void bench_shm_open_4k(void) {
    const size_t iterations = 100000;
    const size_t size = 4096;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_t creator;
    fc_spsc_shm_create(&creator, name, size);

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_spsc_shm_t shm;
        fc_spsc_shm_open(&shm, name, size);
        fc_spsc_shm_close(&shm);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_open_4KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_spsc_shm_close(&creator);
    fc_spsc_shm_unlink(name);
}

static void bench_shm_write_sequential_4k(void) {
    const size_t iterations = 1000000;
    const size_t size = 4096;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_t shm;
    fc_spsc_shm_create(&shm, name, size);
    uint8_t* base = (uint8_t*)fc_spsc_shm_base(&shm);

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        memset(base, (int)(i & 0xFF), size);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_write_seq_4KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_spsc_shm_close(&shm);
    fc_spsc_shm_unlink(name);
}

static void bench_shm_read_sequential_4k(void) {
    const size_t iterations = 1000000;
    const size_t size = 4096;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_t shm;
    fc_spsc_shm_create(&shm, name, size);
    uint8_t* base = (uint8_t*)fc_spsc_shm_base(&shm);
    memset(base, 0xAA, size);

    volatile uint8_t dummy = 0;

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        for (size_t j = 0; j < size; j++) {
            dummy += base[j];
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_read_seq_4KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_spsc_shm_close(&shm);
    fc_spsc_shm_unlink(name);

    (void)dummy;
}

static void bench_shm_copy_between_segments(void) {
    const size_t iterations = 100000;
    const size_t size = 4096;
    char name1[128], name2[256];
    generate_unique_name(name1, sizeof(name1));
    snprintf(name2, sizeof(name2), "%s_2", name1);

    fc_spsc_shm_t shm1, shm2;
    fc_spsc_shm_create(&shm1, name1, size);
    fc_spsc_shm_create(&shm2, name2, size);

    void* base1 = fc_spsc_shm_base(&shm1);
    void* base2 = fc_spsc_shm_base(&shm2);

    memset(base1, 0xAA, size);

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        memcpy(base2, base1, size);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_copy_4KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_spsc_shm_close(&shm2);
    fc_spsc_shm_close(&shm1);
    fc_spsc_shm_unlink(name2);
    fc_spsc_shm_unlink(name1);
}

static void bench_shm_create_open_close_cycle(void) {
    const size_t iterations = 10000;
    const size_t size = 8192;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        fc_spsc_shm_t creator, opener;
        fc_spsc_shm_create(&creator, name, size);
        fc_spsc_shm_open(&opener, name, size);
        fc_spsc_shm_close(&opener);
        fc_spsc_shm_close(&creator);
        fc_spsc_shm_unlink(name);
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);

    fc_bench_result_t result = {
        .name = "shm_full_cycle_8KB",
        .data_size = iterations * size,
        .elapsed_ms = elapsed_ms,
        .iterations = iterations,
        .ops_per_sec = (iterations / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / iterations,
        .bytes_per_op = size,
        .allocs_per_op = 1
    };

    fc_bench_result_print(&result);
}

static void bench_shm_write_cache_lines(void) {
    const size_t iterations = 1000000;
    const size_t size = 4096;
    const size_t cache_line = 64;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_t shm;
    fc_spsc_shm_create(&shm, name, size);
    uint8_t* base = (uint8_t*)fc_spsc_shm_base(&shm);

    fc_bench_time_t start = fc_bench_time_now();

    for (size_t i = 0; i < iterations; i++) {
        for (size_t j = 0; j < size; j += cache_line) {
            base[j] = (uint8_t)(i & 0xFF);
        }
    }

    fc_bench_time_t end = fc_bench_time_now();
    double elapsed_ms = fc_bench_time_elapsed_ms(&start, &end);
    size_t total_writes = iterations * (size / cache_line);

    fc_bench_result_t result = {
        .name = "shm_write_cacheline",
        .data_size = total_writes,
        .elapsed_ms = elapsed_ms,
        .iterations = total_writes,
        .ops_per_sec = (total_writes / elapsed_ms) * 1000.0,
        .mean_ns = (elapsed_ms * 1000000.0) / total_writes,
        .bytes_per_op = 1,
        .allocs_per_op = 0
    };

    fc_bench_result_print(&result);

    fc_spsc_shm_close(&shm);
    fc_spsc_shm_unlink(name);
}

void bench_spsc_shm_run(void) {
    srand((unsigned int)time(NULL));

    printf("\n=== SPSC Shared Memory Benchmarks ===\n\n");

    printf("--- Shared Memory Creation ---\n");
    bench_shm_create_4k();
    bench_shm_create_1m();

    printf("\n--- Shared Memory Open ---\n");
    bench_shm_open_4k();

    printf("\n--- Sequential I/O ---\n");
    bench_shm_write_sequential_4k();
    bench_shm_read_sequential_4k();

    printf("\n--- Data Transfer ---\n");
    bench_shm_copy_between_segments();

    printf("\n--- Lifecycle Operations ---\n");
    bench_shm_create_open_close_cycle();

    printf("\n--- Cache-Aware Access ---\n");
    bench_shm_write_cache_lines();
}
