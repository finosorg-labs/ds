/**
 * @file test_priority_queue.c
 * @brief Unit tests for Priority Queue
 */

#include "test_framework.h"
#include <priority_queue.h>
#include <stdio.h>
#include <stdlib.h>

TEST(test_priority_queue_create_destroy) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);
    FC_TEST_ASSERT(fc_priority_queue_capacity(pq) == 10);
    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 0);
    FC_TEST_ASSERT(fc_priority_queue_is_empty(pq));
    FC_TEST_ASSERT(!fc_priority_queue_is_full(pq));

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_insert_pop) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);

    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 5.0, (void*)5));
    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 1);

    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 3.0, (void*)3));
    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 8.0, (void*)8));
    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 3);

    double priority;
    void* data;

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 8.0);
    FC_TEST_ASSERT(data == (void*)8);

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 5.0);
    FC_TEST_ASSERT(data == (void*)5);

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 3.0);
    FC_TEST_ASSERT(data == (void*)3);

    FC_TEST_ASSERT(fc_priority_queue_is_empty(pq));

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_peek) {
    fc_priority_queue_t* pq = fc_priority_queue_create(5);
    FC_TEST_ASSERT(pq != NULL);

    fc_priority_queue_insert(pq, 10.0, (void*)10);
    fc_priority_queue_insert(pq, 20.0, (void*)20);
    fc_priority_queue_insert(pq, 15.0, (void*)15);

    double priority;
    void* data;

    FC_TEST_ASSERT(fc_priority_queue_peek(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 20.0);
    FC_TEST_ASSERT(data == (void*)20);

    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 3);

    FC_TEST_ASSERT(fc_priority_queue_peek(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 20.0);

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_ordering) {
    fc_priority_queue_t* pq = fc_priority_queue_create(100);
    FC_TEST_ASSERT(pq != NULL);

    double priorities[] = {45.2, 12.3, 78.9, 23.4, 67.8, 34.5, 89.1, 56.7, 90.0, 11.1};
    for (int i = 0; i < 10; i++) {
        FC_TEST_ASSERT(fc_priority_queue_insert(pq, priorities[i], (void*)(intptr_t)i));
    }

    double prev_priority = 1000.0;
    for (int i = 0; i < 10; i++) {
        double priority;
        void* data;
        FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
        FC_TEST_ASSERT(priority <= prev_priority);
        prev_priority = priority;
    }

    FC_TEST_ASSERT(fc_priority_queue_is_empty(pq));

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_full) {
    fc_priority_queue_t* pq = fc_priority_queue_create(3);
    FC_TEST_ASSERT(pq != NULL);

    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 1.0, NULL));
    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 2.0, NULL));
    FC_TEST_ASSERT(fc_priority_queue_insert(pq, 3.0, NULL));

    FC_TEST_ASSERT(fc_priority_queue_is_full(pq));

    FC_TEST_ASSERT(!fc_priority_queue_insert(pq, 4.0, NULL));

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_clear) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);

    for (int i = 0; i < 5; i++) {
        fc_priority_queue_insert(pq, (double)i, NULL);
    }

    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 5);

    fc_priority_queue_clear(pq);

    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 0);
    FC_TEST_ASSERT(fc_priority_queue_is_empty(pq));

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_heapify) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);

    fc_pq_element_t elements[] = {
        {5.0, (void*)5},
        {3.0, (void*)3},
        {8.0, (void*)8},
        {1.0, (void*)1},
        {9.0, (void*)9},
        {2.0, (void*)2}
    };

    FC_TEST_ASSERT(fc_priority_queue_heapify(pq, elements, 6));
    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 6);

    double prev_priority = 1000.0;
    for (int i = 0; i < 6; i++) {
        double priority;
        void* data;
        FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
        FC_TEST_ASSERT(priority <= prev_priority);
        prev_priority = priority;
    }

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_same_priorities) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);

    for (int i = 0; i < 5; i++) {
        fc_priority_queue_insert(pq, 5.0, (void*)(intptr_t)i);
    }

    FC_TEST_ASSERT(fc_priority_queue_size(pq) == 5);

    for (int i = 0; i < 5; i++) {
        double priority;
        void* data;
        FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
        FC_TEST_ASSERT(priority == 5.0);
    }

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_negative_priorities) {
    fc_priority_queue_t* pq = fc_priority_queue_create(10);
    FC_TEST_ASSERT(pq != NULL);

    fc_priority_queue_insert(pq, -5.0, (void*)-5);
    fc_priority_queue_insert(pq, 0.0, (void*)0);
    fc_priority_queue_insert(pq, -10.0, (void*)-10);
    fc_priority_queue_insert(pq, 5.0, (void*)5);

    double priority;
    void* data;

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 5.0);

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == 0.0);

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == -5.0);

    FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
    FC_TEST_ASSERT(priority == -10.0);

    fc_priority_queue_destroy(pq);
}

TEST(test_priority_queue_null_handling) {
    FC_TEST_ASSERT(fc_priority_queue_create(0) == NULL);

    fc_priority_queue_destroy(NULL);

    FC_TEST_ASSERT(!fc_priority_queue_insert(NULL, 1.0, NULL));
    FC_TEST_ASSERT(!fc_priority_queue_pop(NULL, NULL, NULL));
    FC_TEST_ASSERT(!fc_priority_queue_peek(NULL, NULL, NULL));

    FC_TEST_ASSERT(fc_priority_queue_size(NULL) == 0);
    FC_TEST_ASSERT(fc_priority_queue_capacity(NULL) == 0);
    FC_TEST_ASSERT(fc_priority_queue_is_empty(NULL));
    FC_TEST_ASSERT(!fc_priority_queue_is_full(NULL));

    fc_priority_queue_clear(NULL);

    FC_TEST_ASSERT(!fc_priority_queue_heapify(NULL, NULL, 0));
}

TEST(test_priority_queue_large_scale) {
    const size_t n = 1000;
    fc_priority_queue_t* pq = fc_priority_queue_create(n);
    FC_TEST_ASSERT(pq != NULL);

    for (size_t i = 0; i < n; i++) {
        double priority = (double)(n - i);
        FC_TEST_ASSERT(fc_priority_queue_insert(pq, priority, (void*)i));
    }

    FC_TEST_ASSERT(fc_priority_queue_size(pq) == n);

    double prev_priority = (double)n + 1.0;
    for (size_t i = 0; i < n; i++) {
        double priority;
        void* data;
        FC_TEST_ASSERT(fc_priority_queue_pop(pq, &priority, &data));
        FC_TEST_ASSERT(priority <= prev_priority);
        prev_priority = priority;
    }

    FC_TEST_ASSERT(fc_priority_queue_is_empty(pq));

    fc_priority_queue_destroy(pq);
}

void test_priority_queue_register(void) {
    RUN_TEST(test_priority_queue_create_destroy);
    RUN_TEST(test_priority_queue_insert_pop);
    RUN_TEST(test_priority_queue_peek);
    RUN_TEST(test_priority_queue_ordering);
    RUN_TEST(test_priority_queue_full);
    RUN_TEST(test_priority_queue_clear);
    RUN_TEST(test_priority_queue_heapify);
    RUN_TEST(test_priority_queue_same_priorities);
    RUN_TEST(test_priority_queue_negative_priorities);
    RUN_TEST(test_priority_queue_null_handling);
    RUN_TEST(test_priority_queue_large_scale);
}

