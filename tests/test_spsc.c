/**
 * @file test_spsc.c
 * @brief Unit tests for SPSC ring buffer
 */

#include "spsc.h"
#include "test_framework.h"
#include <mem_aligned.h>
#include <string.h>
#include <pthread.h>

TEST(test_spsc_arena_size) {
    size_t size = fc_spsc_arena_size(1024, 64);
    ASSERT_TRUE(size > 0);

    /* Invalid capacity (not power of 2) */
    size = fc_spsc_arena_size(1000, 64);
    ASSERT_EQ(size, 0);

    /* Zero elem size */
    size = fc_spsc_arena_size(1024, 0);
    ASSERT_EQ(size, 0);
}

TEST(test_spsc_init_invalid) {
    fc_spsc_t q;

    /* NULL handle */
    fc_status_t status = fc_spsc_init(NULL, NULL, 0, 16, 8, FC_SPSC_BACKPRESSURE_SPIN);
    ASSERT_EQ(status, FC_ERR_INVALID_ARG);

    /* Invalid capacity */
    size_t size = fc_spsc_arena_size(16, 8);
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    status = fc_spsc_init(&q, mem, size, 15, 8, FC_SPSC_BACKPRESSURE_SPIN);
    ASSERT_EQ(status, FC_ERR_INVALID_ARG);

    fc_aligned_free(mem);
}

TEST(test_spsc_init_attach) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(64, sizeof(int64_t));
    ASSERT_TRUE(size > 0);

    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    /* Initialize (producer side) */
    fc_status_t status = fc_spsc_init(&producer, mem, size, 64, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(fc_spsc_capacity(&producer), 64);
    ASSERT_EQ(fc_spsc_elem_size(&producer), sizeof(int64_t));

    /* Attach (consumer side) */
    status = fc_spsc_attach(&consumer, mem, size);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(fc_spsc_capacity(&consumer), 64);
    ASSERT_EQ(fc_spsc_elem_size(&consumer), sizeof(int64_t));

    fc_aligned_free(mem);
}

TEST(test_spsc_push_pop_single) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(16, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 16, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push single element */
    int64_t elem = 42;
    fc_status_t status = fc_spsc_push(&producer, &elem);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(fc_spsc_len(&producer), 1);

    /* Pop single element */
    int64_t result;
    status = fc_spsc_pop(&consumer, &result);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(result, 42);
    ASSERT_EQ(fc_spsc_len(&consumer), 0);

    fc_aligned_free(mem);
}

TEST(test_spsc_push_pop_sequence) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(128, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 128, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push sequence */
    const int count = 100;
    for (int i = 0; i < count; i++) {
        int64_t elem = i;
        fc_status_t status = fc_spsc_push(&producer, &elem);
        ASSERT_EQ(status, FC_OK);
    }

    /* Pop and verify sequence */
    for (int i = 0; i < count; i++) {
        int64_t result;
        fc_status_t status = fc_spsc_pop(&consumer, &result);
        ASSERT_EQ(status, FC_OK);
        ASSERT_EQ(result, i);
    }

    fc_aligned_free(mem);
}

TEST(test_spsc_pop_empty) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(16, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 16, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Pop from empty queue */
    int64_t result;
    fc_status_t status = fc_spsc_pop(&consumer, &result);
    ASSERT_EQ(status, FC_ERR_WOULD_BLOCK);

    fc_aligned_free(mem);
}

TEST(test_spsc_push_full_spin) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(4, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 4, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Fill the queue */
    for (int i = 0; i < 4; i++) {
        int64_t elem = i;
        fc_status_t status = fc_spsc_push(&producer, &elem);
        ASSERT_EQ(status, FC_OK);
    }

    ASSERT_EQ(fc_spsc_len(&producer), 4);

    /* Try to push one more (should block) */
    int64_t elem = 999;
    fc_status_t status = fc_spsc_push(&producer, &elem);
    ASSERT_EQ(status, FC_ERR_WOULD_BLOCK);

    fc_aligned_free(mem);
}

TEST(test_spsc_push_full_drop) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(4, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 4, sizeof(int64_t), FC_SPSC_BACKPRESSURE_DROP);
    fc_spsc_attach(&consumer, mem, size);

    /* Fill the queue */
    for (int i = 0; i < 4; i++) {
        int64_t elem = i;
        fc_spsc_push(&producer, &elem);
    }

    /* Push more (should drop) */
    uint64_t dropped_before = fc_spsc_dropped(&producer);
    int64_t elem = 999;
    fc_status_t status = fc_spsc_push(&producer, &elem);
    ASSERT_EQ(status, FC_OK);  /* Not an error, just dropped */

    uint64_t dropped_after = fc_spsc_dropped(&producer);
    ASSERT_EQ(dropped_after, dropped_before + 1);

    fc_aligned_free(mem);
}

TEST(test_spsc_bulk_push_pop) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(128, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 128, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push bulk */
    int64_t push_elems[50];
    for (int i = 0; i < 50; i++) {
        push_elems[i] = i;
    }

    size_t pushed = fc_spsc_push_bulk(&producer, push_elems, 50);
    ASSERT_EQ(pushed, 50);
    ASSERT_EQ(fc_spsc_len(&producer), 50);

    /* Pop bulk */
    int64_t pop_elems[50];
    size_t popped = fc_spsc_pop_bulk(&consumer, pop_elems, 50);
    ASSERT_EQ(popped, 50);

    /* Verify values */
    for (int i = 0; i < 50; i++) {
        ASSERT_EQ(pop_elems[i], i);
    }

    ASSERT_EQ(fc_spsc_len(&consumer), 0);

    fc_aligned_free(mem);
}

TEST(test_spsc_bulk_push_partial) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(8, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 8, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Try to push more than capacity */
    int64_t elems[20];
    for (int i = 0; i < 20; i++) {
        elems[i] = i;
    }

    size_t pushed = fc_spsc_push_bulk(&producer, elems, 20);
    ASSERT_EQ(pushed, 8);  /* Only capacity fits */

    fc_aligned_free(mem);
}

TEST(test_spsc_high_watermark) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(16, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 16, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push several elements */
    for (int i = 0; i < 10; i++) {
        int64_t elem = i;
        fc_spsc_push(&producer, &elem);
    }

    /* Pop one (should update high watermark) */
    int64_t result;
    fc_spsc_pop(&consumer, &result);

    uint64_t hwm = fc_spsc_high_watermark(&consumer);
    ASSERT_TRUE(hwm >= 9);

    fc_aligned_free(mem);
}

TEST(test_spsc_wrap_around) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(8, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 8, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push and pop multiple times to wrap around */
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < 5; i++) {
            int64_t elem = i;
            fc_spsc_push(&producer, &elem);
        }

        for (int i = 0; i < 5; i++) {
            int64_t result;
            fc_status_t status = fc_spsc_pop(&consumer, &result);
            ASSERT_EQ(status, FC_OK);
            ASSERT_EQ(result, i);
        }
    }

    fc_aligned_free(mem);
}

/* Thread test data */
typedef struct {
    fc_spsc_t* queue;
    int count;
    int* success;
} thread_test_data_t;

static void* producer_thread(void* arg) {
    thread_test_data_t* data = (thread_test_data_t*)arg;

    for (int i = 0; i < data->count; i++) {
        int64_t elem = i;
        while (fc_spsc_push(data->queue, &elem) != FC_OK) {
            /* Spin */
        }
    }

    return NULL;
}

static void* consumer_thread(void* arg) {
    thread_test_data_t* data = (thread_test_data_t*)arg;
    int received = 0;
    int64_t expected = 0;

    while (received < data->count) {
        int64_t result;
        if (fc_spsc_pop(data->queue, &result) == FC_OK) {
            if (result != expected) {
                *data->success = 0;
                return NULL;
            }
            expected++;
            received++;
        }
    }

    *data->success = (received == data->count);
    return NULL;
}

TEST(test_spsc_concurrent) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(1024, sizeof(int64_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 1024, sizeof(int64_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    int success = 0;
    const int count = 100000;

    thread_test_data_t producer_data = {&producer, count, &success};
    thread_test_data_t consumer_data = {&consumer, count, &success};

    pthread_t prod_thread, cons_thread;

    pthread_create(&prod_thread, NULL, producer_thread, &producer_data);
    pthread_create(&cons_thread, NULL, consumer_thread, &consumer_data);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    ASSERT_TRUE(success);
    ASSERT_EQ(fc_spsc_len(&consumer), 0);

    fc_aligned_free(mem);
}

/* Custom struct test */
typedef struct {
    int64_t id;
    double value;
    char name[16];
} test_struct_t;

TEST(test_spsc_custom_struct) {
    fc_spsc_t producer, consumer;

    size_t size = fc_spsc_arena_size(32, sizeof(test_struct_t));
    void* mem = fc_aligned_alloc(size, 64);
    ASSERT_NOT_NULL(mem);

    fc_spsc_init(&producer, mem, size, 32, sizeof(test_struct_t), FC_SPSC_BACKPRESSURE_SPIN);
    fc_spsc_attach(&consumer, mem, size);

    /* Push custom struct */
    test_struct_t elem = {42, 3.14, "test"};
    fc_status_t status = fc_spsc_push(&producer, &elem);
    ASSERT_EQ(status, FC_OK);

    /* Pop and verify */
    test_struct_t result;
    status = fc_spsc_pop(&consumer, &result);
    ASSERT_EQ(status, FC_OK);
    ASSERT_EQ(result.id, 42);
    FC_TEST_ASSERT_DOUBLE_EQ(result.value, 3.14, 1e-9);
    FC_TEST_ASSERT_STR_EQ(result.name, "test");

    fc_aligned_free(mem);
}

void register_spsc_tests(void) {
    RUN_TEST(test_spsc_arena_size);
    RUN_TEST(test_spsc_init_invalid);
    RUN_TEST(test_spsc_init_attach);
    RUN_TEST(test_spsc_push_pop_single);
    RUN_TEST(test_spsc_push_pop_sequence);
    RUN_TEST(test_spsc_pop_empty);
    RUN_TEST(test_spsc_push_full_spin);
    RUN_TEST(test_spsc_push_full_drop);
    RUN_TEST(test_spsc_bulk_push_pop);
    RUN_TEST(test_spsc_bulk_push_partial);
    RUN_TEST(test_spsc_high_watermark);
    RUN_TEST(test_spsc_wrap_around);
    RUN_TEST(test_spsc_concurrent);
    RUN_TEST(test_spsc_custom_struct);
}
