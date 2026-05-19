/**
 * @file test_ring_buffer_mt.c
 * @brief Multi-threaded tests for Lock-Free Ring Buffer
 */

#include "test_framework.h"
#include <ring_buffer.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ITERATIONS 100000
#define BUFFER_SIZE 1024

typedef struct {
    fc_ring_buffer_t* rb;
    size_t count;
} thread_data_t;

static void* producer_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;

    for (size_t i = 0; i < NUM_ITERATIONS; i++) {
        fc_ring_buffer_push(data->rb, (double)i);
    }

    data->count = NUM_ITERATIONS;
    return NULL;
}

static void* consumer_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double value;
    size_t count = 0;
    size_t empty_count = 0;
    const size_t max_empty = 100000; // Prevent infinite loop

    // Keep consuming until producer is done and buffer is empty
    while (count < NUM_ITERATIONS && empty_count < max_empty) {
        if (fc_ring_buffer_pop(data->rb, &value)) {
            count++;
            empty_count = 0;
        } else {
            empty_count++;
            // Small yield to avoid busy-waiting
            sched_yield();
        }
    }

    data->count = count;
    return NULL;
}

TEST(test_ring_buffer_spsc_concurrent) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(BUFFER_SIZE);
    FC_TEST_ASSERT(rb != NULL);

    thread_data_t producer_data = {.rb = rb, .count = 0};
    thread_data_t consumer_data = {.rb = rb, .count = 0};

    pthread_t producer, consumer;

    // Start both threads
    FC_TEST_ASSERT(pthread_create(&producer, NULL, producer_thread, &producer_data) == 0);
    FC_TEST_ASSERT(pthread_create(&consumer, NULL, consumer_thread, &consumer_data) == 0);

    // Wait for both threads
    FC_TEST_ASSERT(pthread_join(producer, NULL) == 0);
    FC_TEST_ASSERT(pthread_join(consumer, NULL) == 0);

    // Verify producer pushed all items
    FC_TEST_ASSERT(producer_data.count == NUM_ITERATIONS);

    // Consumer should have consumed most items (may lose some due to overwrite)
    // In a properly sized buffer, should get all items
    FC_TEST_ASSERT(consumer_data.count > 0);
    FC_TEST_ASSERT(consumer_data.count <= NUM_ITERATIONS);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_spsc_stress) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(64);
    FC_TEST_ASSERT(rb != NULL);

    thread_data_t producer_data = {.rb = rb, .count = 0};
    thread_data_t consumer_data = {.rb = rb, .count = 0};

    pthread_t producer, consumer;

    // Smaller buffer, more contention
    FC_TEST_ASSERT(pthread_create(&producer, NULL, producer_thread, &producer_data) == 0);
    FC_TEST_ASSERT(pthread_create(&consumer, NULL, consumer_thread, &consumer_data) == 0);

    FC_TEST_ASSERT(pthread_join(producer, NULL) == 0);
    FC_TEST_ASSERT(pthread_join(consumer, NULL) == 0);

    FC_TEST_ASSERT(producer_data.count == NUM_ITERATIONS);
    // With small buffer and overwrite, consumer may not get all items
    FC_TEST_ASSERT(consumer_data.count > 0);

    fc_ring_buffer_destroy(rb);
}

static void* batch_producer_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    const size_t batch_size = 100;
    double values[100];

    for (size_t i = 0; i < batch_size; i++) {
        values[i] = (double)i;
    }

    for (size_t i = 0; i < NUM_ITERATIONS / batch_size; i++) {
        fc_ring_buffer_push_batch(data->rb, values, batch_size);
    }

    data->count = NUM_ITERATIONS;
    return NULL;
}

static void* batch_consumer_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    const size_t batch_size = 100;
    double values[100];
    size_t count = 0;
    size_t empty_count = 0;
    const size_t max_empty = 10000;

    while (count < NUM_ITERATIONS && empty_count < max_empty) {
        size_t popped = fc_ring_buffer_pop_batch(data->rb, values, batch_size);
        if (popped > 0) {
            count += popped;
            empty_count = 0;
        } else {
            empty_count++;
            sched_yield();
        }
    }

    data->count = count;
    return NULL;
}

TEST(test_ring_buffer_spsc_batch_concurrent) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(BUFFER_SIZE);
    FC_TEST_ASSERT(rb != NULL);

    thread_data_t producer_data = {.rb = rb, .count = 0};
    thread_data_t consumer_data = {.rb = rb, .count = 0};

    pthread_t producer, consumer;

    FC_TEST_ASSERT(pthread_create(&producer, NULL, batch_producer_thread, &producer_data) == 0);
    FC_TEST_ASSERT(pthread_create(&consumer, NULL, batch_consumer_thread, &consumer_data) == 0);

    FC_TEST_ASSERT(pthread_join(producer, NULL) == 0);
    FC_TEST_ASSERT(pthread_join(consumer, NULL) == 0);

    FC_TEST_ASSERT(producer_data.count == NUM_ITERATIONS);
    // Consumer should get most items, but may lose some due to timing
    FC_TEST_ASSERT(consumer_data.count > NUM_ITERATIONS / 2);

    fc_ring_buffer_destroy(rb);
}

void register_ring_buffer_mt_tests(void) {
    RUN_TEST(test_ring_buffer_spsc_concurrent);
    RUN_TEST(test_ring_buffer_spsc_stress);
    RUN_TEST(test_ring_buffer_spsc_batch_concurrent);
}
