/**
 * @file test_mem_pool.c
 * @brief Unit tests for memory pool
 */

#include "mem_pool.h"
#include "test_framework.h"
#include <pthread.h>
#include <string.h>

TEST(test_mem_pool_create_destroy) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(1024, 100);
    ASSERT_NOT_NULL(pool);
    ASSERT_EQ(fc_ds_mem_pool_capacity(pool), 100);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), 100);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);
    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_create_invalid_args) {
    fc_mem_pool_t* pool1 = fc_ds_mem_pool_create(0, 100);
    ASSERT_NULL(pool1);

    fc_mem_pool_t* pool2 = fc_ds_mem_pool_create(1024, 0);
    ASSERT_NULL(pool2);
}

TEST(test_mem_pool_alloc_free_single) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(1024, 10);
    ASSERT_NOT_NULL(pool);

    void* ptr = fc_ds_mem_pool_alloc(pool);
    ASSERT_NOT_NULL(ptr);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 1);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), 9);

    memset(ptr, 0xAB, 1024);

    fc_ds_mem_pool_free(pool, ptr);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), 10);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_alloc_all_blocks) {
    const size_t num_blocks = 50;
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(512, num_blocks);
    ASSERT_NOT_NULL(pool);

    void* ptrs[50];
    for (size_t i = 0; i < num_blocks; i++) {
        ptrs[i] = fc_ds_mem_pool_alloc(pool);
        ASSERT_NOT_NULL(ptrs[i]);
    }

    ASSERT_EQ(fc_ds_mem_pool_used(pool), num_blocks);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), 0);

    void* ptr_overflow = fc_ds_mem_pool_alloc(pool);
    ASSERT_NULL(ptr_overflow);

    for (size_t i = 0; i < num_blocks; i++) {
        fc_ds_mem_pool_free(pool, ptrs[i]);
    }

    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), num_blocks);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_alloc_free_pattern) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(256, 20);
    ASSERT_NOT_NULL(pool);

    void* ptr1 = fc_ds_mem_pool_alloc(pool);
    void* ptr2 = fc_ds_mem_pool_alloc(pool);
    void* ptr3 = fc_ds_mem_pool_alloc(pool);

    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_NOT_NULL(ptr3);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 3);

    fc_ds_mem_pool_free(pool, ptr2);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 2);

    void* ptr4 = fc_ds_mem_pool_alloc(pool);
    ASSERT_NOT_NULL(ptr4);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 3);

    fc_ds_mem_pool_free(pool, ptr1);
    fc_ds_mem_pool_free(pool, ptr3);
    fc_ds_mem_pool_free(pool, ptr4);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_batch_alloc_free) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(128, 100);
    ASSERT_NOT_NULL(pool);

    void* ptrs[50];
    size_t allocated = fc_ds_mem_pool_alloc_batch(pool, 50, ptrs);
    ASSERT_EQ(allocated, 50);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 50);

    for (size_t i = 0; i < allocated; i++) {
        ASSERT_NOT_NULL(ptrs[i]);
    }

    fc_ds_mem_pool_free_batch(pool, (void* const*)ptrs, allocated);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_batch_partial_alloc) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(64, 30);
    ASSERT_NOT_NULL(pool);

    void* ptrs[50];
    size_t allocated = fc_ds_mem_pool_alloc_batch(pool, 50, ptrs);
    ASSERT_EQ(allocated, 30);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 30);
    ASSERT_EQ(fc_ds_mem_pool_available(pool), 0);

    fc_ds_mem_pool_free_batch(pool, (void* const*)ptrs, allocated);
    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_stats) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(512, 100);
    ASSERT_NOT_NULL(pool);

    fc_mem_pool_stats_t stats;
    fc_status_t status = fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(stats.block_size, fc_ds_mem_pool_block_size(pool));
    ASSERT_EQ(stats.total_blocks, 100);
    ASSERT_EQ(stats.used_blocks, 0);
    ASSERT_EQ(stats.available_blocks, 100);
    ASSERT_EQ(stats.peak_usage, 0);
    ASSERT_EQ(stats.alloc_count, 0);
    ASSERT_EQ(stats.free_count, 0);

    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = fc_ds_mem_pool_alloc(pool);
    }

    status = fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(stats.used_blocks, 10);
    ASSERT_EQ(stats.available_blocks, 90);
    ASSERT_EQ(stats.peak_usage, 10);
    ASSERT_EQ(stats.alloc_count, 10);

    for (int i = 0; i < 5; i++) {
        fc_ds_mem_pool_free(pool, ptrs[i]);
    }

    status = fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(stats.used_blocks, 5);
    ASSERT_EQ(stats.peak_usage, 10);
    ASSERT_EQ(stats.free_count, 5);

    for (int i = 5; i < 10; i++) {
        fc_ds_mem_pool_free(pool, ptrs[i]);
    }

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_reset_stats) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(256, 50);
    ASSERT_NOT_NULL(pool);

    void* ptrs[20];
    for (int i = 0; i < 20; i++) {
        ptrs[i] = fc_ds_mem_pool_alloc(pool);
    }

    fc_mem_pool_stats_t stats;
    fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(stats.peak_usage, 20);
    ASSERT_EQ(stats.alloc_count, 20);

    fc_ds_mem_pool_reset_stats(pool);

    fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(stats.peak_usage, 0);
    ASSERT_EQ(stats.alloc_count, 0);
    ASSERT_EQ(stats.free_count, 0);
    ASSERT_EQ(stats.used_blocks, 20);

    for (int i = 0; i < 20; i++) {
        fc_ds_mem_pool_free(pool, ptrs[i]);
    }

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_block_size_alignment) {
    fc_mem_pool_t* pool = fc_ds_mem_pool_create(100, 10);
    ASSERT_NOT_NULL(pool);

    size_t block_size = fc_ds_mem_pool_block_size(pool);
    FC_TEST_ASSERT(block_size >= 100);
    ASSERT_EQ(block_size % 64, 0);

    fc_ds_mem_pool_destroy(pool);
}

typedef struct {
    fc_mem_pool_t* pool;
    int thread_id;
    int iterations;
    int errors;
} thread_test_args_t;

static void* thread_alloc_free_worker(void* arg) {
    thread_test_args_t* args = (thread_test_args_t*)arg;
    args->errors = 0;

    for (int i = 0; i < args->iterations; i++) {
        void* ptr = fc_ds_mem_pool_alloc(args->pool);
        if (!ptr) {
            args->errors++;
            continue;
        }

        memset(ptr, args->thread_id & 0xFF, 64);

        fc_ds_mem_pool_free(args->pool, ptr);
    }

    return NULL;
}

TEST(test_mem_pool_thread_safety) {
    const int num_threads = 4;
    const int iterations = 1000;

    fc_mem_pool_t* pool = fc_ds_mem_pool_create(128, 200);
    ASSERT_NOT_NULL(pool);

    pthread_t threads[4];
    thread_test_args_t args[4];

    for (int i = 0; i < num_threads; i++) {
        args[i].pool = pool;
        args[i].thread_id = i;
        args[i].iterations = iterations;
        args[i].errors = 0;
        pthread_create(&threads[i], NULL, thread_alloc_free_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);

    // Verify no errors occurred in any thread
    for (int i = 0; i < num_threads; i++) {
        ASSERT_EQ(args[i].errors, 0);
    }

    fc_mem_pool_stats_t stats;
    fc_ds_mem_pool_get_stats(pool, &stats);
    ASSERT_EQ(stats.alloc_count, stats.free_count);

    fc_ds_mem_pool_destroy(pool);
}

static void* thread_batch_worker(void* arg) {
    thread_test_args_t* args = (thread_test_args_t*)arg;
    args->errors = 0;

    for (int i = 0; i < args->iterations; i++) {
        void* ptrs[10];
        size_t allocated = fc_ds_mem_pool_alloc_batch(args->pool, 10, ptrs);

        if (allocated > 0) {
            fc_ds_mem_pool_free_batch(args->pool, (void* const*)ptrs, allocated);
        }
    }

    return NULL;
}

TEST(test_mem_pool_thread_safety_batch) {
    const int num_threads = 4;
    const int iterations = 500;

    fc_mem_pool_t* pool = fc_ds_mem_pool_create(256, 500);
    ASSERT_NOT_NULL(pool);

    pthread_t threads[4];
    thread_test_args_t args[4];

    for (int i = 0; i < num_threads; i++) {
        args[i].pool = pool;
        args[i].thread_id = i;
        args[i].iterations = iterations;
        args[i].errors = 0;
        pthread_create(&threads[i], NULL, thread_batch_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    ASSERT_EQ(fc_ds_mem_pool_used(pool), 0);

    fc_ds_mem_pool_destroy(pool);
}

TEST(test_mem_pool_null_handling) {
    fc_ds_mem_pool_destroy(NULL);

    void* ptr = fc_ds_mem_pool_alloc(NULL);
    ASSERT_NULL(ptr);

    fc_ds_mem_pool_free(NULL, (void*)0x1234);

    size_t result = fc_ds_mem_pool_alloc_batch(NULL, 10, NULL);
    ASSERT_EQ(result, 0);

    fc_ds_mem_pool_free_batch(NULL, NULL, 10);

    ASSERT_EQ(fc_ds_mem_pool_available(NULL), 0);
    ASSERT_EQ(fc_ds_mem_pool_used(NULL), 0);
    ASSERT_EQ(fc_ds_mem_pool_capacity(NULL), 0);
    ASSERT_EQ(fc_ds_mem_pool_block_size(NULL), 0);

    fc_mem_pool_stats_t stats;
    fc_status_t status = fc_ds_mem_pool_get_stats(NULL, &stats);
    ASSERT_EQ(status, FC_ERR_INVALID_ARG);

    fc_ds_mem_pool_reset_stats(NULL);
}

void register_mem_pool_tests(void) {
    RUN_TEST(test_mem_pool_create_destroy);
    RUN_TEST(test_mem_pool_create_invalid_args);
    RUN_TEST(test_mem_pool_alloc_free_single);
    RUN_TEST(test_mem_pool_alloc_all_blocks);
    RUN_TEST(test_mem_pool_alloc_free_pattern);
    RUN_TEST(test_mem_pool_batch_alloc_free);
    RUN_TEST(test_mem_pool_batch_partial_alloc);
    RUN_TEST(test_mem_pool_stats);
    RUN_TEST(test_mem_pool_reset_stats);
    RUN_TEST(test_mem_pool_block_size_alignment);
    RUN_TEST(test_mem_pool_thread_safety);
    RUN_TEST(test_mem_pool_thread_safety_batch);
    RUN_TEST(test_mem_pool_null_handling);
}
