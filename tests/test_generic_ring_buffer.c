/**
 * @file test_generic_ring_buffer.c
 * @brief Unit tests for Generic Ring Buffer
 */

#include "test_framework.h"
#include <generic_ring_buffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    double value;
    char name[16];
} test_struct_t;

TEST(test_generic_ring_buffer_create_destroy) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(10, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);
    FC_TEST_ASSERT(fc_generic_ring_buffer_capacity(rb) >= 10);
    FC_TEST_ASSERT(fc_generic_ring_buffer_element_size(rb) == sizeof(int));
    FC_TEST_ASSERT(fc_generic_ring_buffer_is_empty(rb));
    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_push_pop_int) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(4, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    int values[] = {10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_push(rb, &values[i]));
    }

    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 4);
    FC_TEST_ASSERT(fc_generic_ring_buffer_is_full(rb));

    int out;
    for (int i = 0; i < 4; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_pop(rb, &out));
        FC_TEST_ASSERT(out == values[i]);
    }

    FC_TEST_ASSERT(fc_generic_ring_buffer_is_empty(rb));

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_push_pop_struct) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(4, sizeof(test_struct_t));
    FC_TEST_ASSERT(rb != NULL);

    test_struct_t values[] = {
        {1, 1.1, "first"},
        {2, 2.2, "second"},
        {3, 3.3, "third"},
        {4, 4.4, "fourth"}
    };

    for (int i = 0; i < 4; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_push(rb, &values[i]));
    }

    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 4);

    test_struct_t out;
    for (int i = 0; i < 4; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_pop(rb, &out));
        FC_TEST_ASSERT(out.id == values[i].id);
        FC_TEST_ASSERT(out.value == values[i].value);
        FC_TEST_ASSERT(strcmp(out.name, values[i].name) == 0);
    }

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_overwrite) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(4, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    for (int i = 0; i < 8; i++) {
        fc_generic_ring_buffer_push(rb, &i);
    }

    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 4);

    int out;
    for (int i = 0; i < 4; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_pop(rb, &out));
        FC_TEST_ASSERT(out == i + 4);
    }

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_batch_operations) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(8, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    int values[] = {1, 2, 3, 4, 5};
    size_t pushed = fc_generic_ring_buffer_push_batch(rb, values, 5);
    FC_TEST_ASSERT(pushed == 5);
    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 5);

    int out[5];
    size_t popped = fc_generic_ring_buffer_pop_batch(rb, out, 5);
    FC_TEST_ASSERT(popped == 5);

    for (int i = 0; i < 5; i++) {
        FC_TEST_ASSERT(out[i] == values[i]);
    }

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_get) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(8, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    int values[] = {10, 20, 30, 40, 50};
    fc_generic_ring_buffer_push_batch(rb, values, 5);

    int out;
    for (int i = 0; i < 5; i++) {
        FC_TEST_ASSERT(fc_generic_ring_buffer_get(rb, i, &out));
        FC_TEST_ASSERT(out == values[i]);
    }

    FC_TEST_ASSERT(!fc_generic_ring_buffer_get(rb, 5, &out));

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_get_all) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(8, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    int values[] = {10, 20, 30, 40, 50};
    fc_generic_ring_buffer_push_batch(rb, values, 5);

    int out[5];
    size_t count = fc_generic_ring_buffer_get_all(rb, out);
    FC_TEST_ASSERT(count == 5);

    for (int i = 0; i < 5; i++) {
        FC_TEST_ASSERT(out[i] == values[i]);
    }

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_clear) {
    fc_generic_ring_buffer_t* rb = fc_generic_ring_buffer_create(8, sizeof(int));
    FC_TEST_ASSERT(rb != NULL);

    int values[] = {1, 2, 3, 4, 5};
    fc_generic_ring_buffer_push_batch(rb, values, 5);
    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 5);

    fc_generic_ring_buffer_clear(rb);
    FC_TEST_ASSERT(fc_generic_ring_buffer_is_empty(rb));
    FC_TEST_ASSERT(fc_generic_ring_buffer_size(rb) == 0);

    fc_generic_ring_buffer_destroy(rb);
}

TEST(test_generic_ring_buffer_null_checks) {
    FC_TEST_ASSERT(fc_generic_ring_buffer_create(0, sizeof(int)) == NULL);
    FC_TEST_ASSERT(fc_generic_ring_buffer_create(10, 0) == NULL);

    fc_generic_ring_buffer_destroy(NULL);

    FC_TEST_ASSERT(!fc_generic_ring_buffer_push(NULL, NULL));
    FC_TEST_ASSERT(!fc_generic_ring_buffer_pop(NULL, NULL));
    FC_TEST_ASSERT(fc_generic_ring_buffer_size(NULL) == 0);
    FC_TEST_ASSERT(fc_generic_ring_buffer_capacity(NULL) == 0);
    FC_TEST_ASSERT(fc_generic_ring_buffer_element_size(NULL) == 0);
}

void register_generic_ring_buffer_tests(void) {
    RUN_TEST(test_generic_ring_buffer_create_destroy);
    RUN_TEST(test_generic_ring_buffer_push_pop_int);
    RUN_TEST(test_generic_ring_buffer_push_pop_struct);
    RUN_TEST(test_generic_ring_buffer_overwrite);
    RUN_TEST(test_generic_ring_buffer_batch_operations);
    RUN_TEST(test_generic_ring_buffer_get);
    RUN_TEST(test_generic_ring_buffer_get_all);
    RUN_TEST(test_generic_ring_buffer_clear);
    RUN_TEST(test_generic_ring_buffer_null_checks);
}


