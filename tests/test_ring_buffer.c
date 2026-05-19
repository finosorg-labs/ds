/**
 * @file test_ring_buffer.c
 * @brief Unit tests for Ring Buffer
 */

#include "test_framework.h"
#include <ring_buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

TEST(test_ring_buffer_create_destroy) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(10);
    FC_TEST_ASSERT(rb != NULL);
    FC_TEST_ASSERT(fc_ring_buffer_capacity(rb) == 16);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 0);
    FC_TEST_ASSERT(fc_ring_buffer_is_empty(rb));
    FC_TEST_ASSERT(!fc_ring_buffer_is_full(rb));

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_push_pop_single) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(4);
    FC_TEST_ASSERT(rb != NULL);

    FC_TEST_ASSERT(fc_ring_buffer_push(rb, 1.0));
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 1);

    FC_TEST_ASSERT(fc_ring_buffer_push(rb, 2.0));
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 2);

    double val;
    FC_TEST_ASSERT(fc_ring_buffer_pop(rb, &val));
    FC_TEST_ASSERT(val == 1.0);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 1);

    FC_TEST_ASSERT(fc_ring_buffer_pop(rb, &val));
    FC_TEST_ASSERT(val == 2.0);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 0);
    FC_TEST_ASSERT(fc_ring_buffer_is_empty(rb));

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_wraparound) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(4);
    FC_TEST_ASSERT(rb != NULL);

    for (int i = 0; i < 10; i++) {
        fc_ring_buffer_push(rb, (double)i);
    }

    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 4);
    FC_TEST_ASSERT(fc_ring_buffer_is_full(rb));

    double val;
    FC_TEST_ASSERT(fc_ring_buffer_pop(rb, &val));
    FC_TEST_ASSERT(val == 6.0);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_batch_operations) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(8);
    FC_TEST_ASSERT(rb != NULL);

    double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    size_t pushed = fc_ring_buffer_push_batch(rb, values, 5);
    FC_TEST_ASSERT(pushed == 5);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 5);

    double out[5];
    size_t popped = fc_ring_buffer_pop_batch(rb, out, 3);
    FC_TEST_ASSERT(popped == 3);
    FC_TEST_ASSERT(out[0] == 1.0 && out[1] == 2.0 && out[2] == 3.0);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 2);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_get_operations) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(8);
    FC_TEST_ASSERT(rb != NULL);

    double values[] = {10.0, 20.0, 30.0, 40.0};
    fc_ring_buffer_push_batch(rb, values, 4);

    double val;
    FC_TEST_ASSERT(fc_ring_buffer_get(rb, 0, &val));
    FC_TEST_ASSERT(val == 10.0);

    FC_TEST_ASSERT(fc_ring_buffer_get(rb, 3, &val));
    FC_TEST_ASSERT(val == 40.0);

    FC_TEST_ASSERT(!fc_ring_buffer_get(rb, 4, &val));

    double all[4];
    size_t count = fc_ring_buffer_get_all(rb, all);
    FC_TEST_ASSERT(count == 4);
    FC_TEST_ASSERT(all[0] == 10.0 && all[1] == 20.0 && all[2] == 30.0 && all[3] == 40.0);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_clear) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(8);
    FC_TEST_ASSERT(rb != NULL);

    double values[] = {1.0, 2.0, 3.0};
    fc_ring_buffer_push_batch(rb, values, 3);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 3);

    fc_ring_buffer_clear(rb);
    FC_TEST_ASSERT(fc_ring_buffer_size(rb) == 0);
    FC_TEST_ASSERT(fc_ring_buffer_is_empty(rb));

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_null_inputs) {
    FC_TEST_ASSERT(fc_ring_buffer_create(0) == NULL);

    fc_ring_buffer_destroy(NULL);

    FC_TEST_ASSERT(!fc_ring_buffer_push(NULL, 1.0));

    double val;
    FC_TEST_ASSERT(!fc_ring_buffer_pop(NULL, &val));

    fc_ring_buffer_t* rb = fc_ring_buffer_create(4);
    FC_TEST_ASSERT(!fc_ring_buffer_pop(rb, NULL));

    FC_TEST_ASSERT(fc_ring_buffer_push_batch(NULL, &val, 1) == 0);
    FC_TEST_ASSERT(fc_ring_buffer_push_batch(rb, NULL, 1) == 0);

    FC_TEST_ASSERT(fc_ring_buffer_size(NULL) == 0);
    FC_TEST_ASSERT(fc_ring_buffer_capacity(NULL) == 0);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_sliding_window) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(5);
    FC_TEST_ASSERT(rb != NULL);

    // Capacity is rounded up to 8 (next power of 2)
    size_t actual_capacity = fc_ring_buffer_capacity(rb);
    double* all = (double*)malloc(actual_capacity * sizeof(double));
    FC_TEST_ASSERT(all != NULL);

    for (int i = 0; i < 20; i++) {
        fc_ring_buffer_push(rb, (double)i);

        // Once buffer is full, verify sliding window behavior
        if (fc_ring_buffer_is_full(rb)) {
            size_t count = fc_ring_buffer_get_all(rb, all);
            FC_TEST_ASSERT(count == actual_capacity);

            // Verify the values are the most recent ones
            int expected_start = i - (int)actual_capacity + 1;
            for (size_t j = 0; j < count; j++) {
                double expected = (double)(expected_start + (int)j);
                FC_TEST_ASSERT(all[j] == expected);
            }
        }
    }

    free(all);
    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_empty_pop) {
    fc_ring_buffer_t* rb = fc_ring_buffer_create(4);
    FC_TEST_ASSERT(rb != NULL);

    double val;
    FC_TEST_ASSERT(!fc_ring_buffer_pop(rb, &val));

    double out[5];
    size_t popped = fc_ring_buffer_pop_batch(rb, out, 5);
    FC_TEST_ASSERT(popped == 0);

    fc_ring_buffer_destroy(rb);
}

TEST(test_ring_buffer_power_of_two) {
    fc_ring_buffer_t* rb1 = fc_ring_buffer_create(1);
    FC_TEST_ASSERT(fc_ring_buffer_capacity(rb1) == 1);
    fc_ring_buffer_destroy(rb1);

    fc_ring_buffer_t* rb2 = fc_ring_buffer_create(3);
    FC_TEST_ASSERT(fc_ring_buffer_capacity(rb2) == 4);
    fc_ring_buffer_destroy(rb2);

    fc_ring_buffer_t* rb3 = fc_ring_buffer_create(100);
    FC_TEST_ASSERT(fc_ring_buffer_capacity(rb3) == 128);
    fc_ring_buffer_destroy(rb3);
}

/*
 * Test Suite Registration
 */
void register_ring_buffer_tests(void) {
    RUN_TEST(test_ring_buffer_create_destroy);
    RUN_TEST(test_ring_buffer_push_pop_single);
    RUN_TEST(test_ring_buffer_wraparound);
    RUN_TEST(test_ring_buffer_batch_operations);
    RUN_TEST(test_ring_buffer_get_operations);
    RUN_TEST(test_ring_buffer_clear);
    RUN_TEST(test_ring_buffer_null_inputs);
    RUN_TEST(test_ring_buffer_sliding_window);
    RUN_TEST(test_ring_buffer_empty_pop);
    RUN_TEST(test_ring_buffer_power_of_two);
}
