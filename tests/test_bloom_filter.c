/**
 * @file test_bloom_filter.c
 * @brief Unit tests for Bloom Filter
 */

#include "test_framework.h"
#include <bloom_filter.h>
#include <platform.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

/* Test optimal size and hash count calculations */
TEST(test_bloom_optimal_calculations) {
    size_t size = fc_bloom_optimal_size(1000, 0.01);
    FC_TEST_ASSERT(size > 0);
    FC_TEST_ASSERT(size % 64 == 0);

    size_t hash_count = fc_bloom_optimal_hash_count(size, 1000);
    FC_TEST_ASSERT(hash_count > 0);
    FC_TEST_ASSERT(hash_count <= 32);

    /* Invalid inputs */
    size = fc_bloom_optimal_size(0, 0.01);
    FC_TEST_ASSERT(size == 0);

    size = fc_bloom_optimal_size(1000, 0.0);
    FC_TEST_ASSERT(size == 0);

    size = fc_bloom_optimal_size(1000, 1.0);
    FC_TEST_ASSERT(size == 0);
}

/* Test basic create and destroy */
TEST(test_bloom_create_destroy) {
    fc_bloom_config_t config = {
        .expected_elements = 1000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    fc_bloom_stats_t stats;
    fc_status_t err = fc_bloom_get_stats(filter, &stats);
    FC_TEST_ASSERT(err == FC_OK);
    FC_TEST_ASSERT(stats.bit_array_size > 0);
    FC_TEST_ASSERT(stats.num_hash_functions > 0);
    FC_TEST_ASSERT(stats.elements_added == 0);
    FC_TEST_ASSERT(stats.memory_bytes > 0);

    fc_bloom_destroy(filter);
}

/* Test explicit create with size and hash count */
TEST(test_bloom_create_explicit) {
    fc_bloom_filter_t* filter = fc_bloom_create_explicit(1024, 7);
    FC_TEST_ASSERT(filter != NULL);

    fc_bloom_stats_t stats;
    fc_status_t err = fc_bloom_get_stats(filter, &stats);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(stats.bit_array_size, 1024);
    ASSERT_EQ(stats.num_hash_functions, 7);

    fc_bloom_destroy(filter);

    /* Invalid parameters */
    filter = fc_bloom_create_explicit(0, 7);
    FC_TEST_ASSERT(filter == NULL);

    filter = fc_bloom_create_explicit(1024, 0);
    FC_TEST_ASSERT(filter == NULL);
}

/* Test create with invalid config */
TEST(test_bloom_create_invalid) {
    fc_bloom_filter_t* filter = fc_bloom_create(NULL);
    FC_TEST_ASSERT(filter == NULL);

    fc_bloom_config_t config = {
        .expected_elements = 0,
        .false_positive_rate = 0.01
    };
    filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter == NULL);

    config.expected_elements = 1000;
    config.false_positive_rate = 0.0;
    filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter == NULL);

    config.false_positive_rate = 1.0;
    filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter == NULL);
}

/* Test basic add and contains operations */
TEST(test_bloom_add_contains_basic) {
    fc_bloom_config_t config = {
        .expected_elements = 100,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    const char* item1 = "hello";
    const char* item2 = "world";
    const char* item3 = "bloom";
    const char* item4 = "filter";

    /* Add items */
    fc_status_t err = fc_bloom_add(filter, item1, strlen(item1));
    ASSERT_EQ(err, FC_OK);

    err = fc_bloom_add(filter, item2, strlen(item2));
    ASSERT_EQ(err, FC_OK);

    /* Check added items */
    bool result;
    err = fc_bloom_contains(filter, item1, strlen(item1), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == true);

    err = fc_bloom_contains(filter, item2, strlen(item2), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == true);

    /* Check non-added items */
    err = fc_bloom_contains(filter, item3, strlen(item3), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == false);

    err = fc_bloom_contains(filter, item4, strlen(item4), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == false);

    /* Verify stats */
    fc_bloom_stats_t stats;
    err = fc_bloom_get_stats(filter, &stats);
    ASSERT_EQ(err, FC_OK);
    ASSERT_EQ(stats.elements_added, 2);

    fc_bloom_destroy(filter);
}

/* Test add and contains with numeric data */
TEST(test_bloom_add_contains_numbers) {
    fc_bloom_config_t config = {
        .expected_elements = 1000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    /* Add numbers 0-99 */
    for (int i = 0; i < 100; i++) {
        fc_status_t err = fc_bloom_add(filter, &i, sizeof(i));
        ASSERT_EQ(err, FC_OK);
    }

    /* Verify all added numbers are found */
    for (int i = 0; i < 100; i++) {
        bool result;
        fc_status_t err = fc_bloom_contains(filter, &i, sizeof(i), &result);
        ASSERT_EQ(err, FC_OK);
        FC_TEST_ASSERT(result == true);
    }

    /* Check false positive rate on non-added numbers */
    int false_positives = 0;
    for (int i = 100; i < 200; i++) {
        bool result;
        fc_status_t err = fc_bloom_contains(filter, &i, sizeof(i), &result);
        ASSERT_EQ(err, FC_OK);
        if (result) {
            false_positives++;
        }
    }

    /* Should have very few false positives */
    FC_TEST_ASSERT(false_positives < 5);

    fc_bloom_destroy(filter);
}

/* Test batch add and contains operations */
TEST(test_bloom_batch_operations) {
    fc_bloom_config_t config = {
        .expected_elements = 100,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    const char* items[] = {"apple", "banana", "cherry", "date", "elderberry"};
    const void* item_ptrs[5];
    size_t lengths[5];

    for (int i = 0; i < 5; i++) {
        item_ptrs[i] = items[i];
        lengths[i] = strlen(items[i]);
    }

    /* Batch add */
    fc_status_t err = fc_bloom_add_batch(filter, item_ptrs, lengths, 5);
    ASSERT_EQ(err, FC_OK);

    /* Batch contains check */
    bool results[5];
    err = fc_bloom_contains_batch(filter, item_ptrs, lengths, 5, results);
    ASSERT_EQ(err, FC_OK);

    for (int i = 0; i < 5; i++) {
        FC_TEST_ASSERT(results[i] == true);
    }

    /* Check non-added items */
    const char* not_added[] = {"fig", "grape", "honeydew"};
    const void* not_added_ptrs[3];
    size_t not_added_lengths[3];

    for (int i = 0; i < 3; i++) {
        not_added_ptrs[i] = not_added[i];
        not_added_lengths[i] = strlen(not_added[i]);
    }

    bool not_added_results[3];
    err = fc_bloom_contains_batch(filter, not_added_ptrs, not_added_lengths, 3, not_added_results);
    ASSERT_EQ(err, FC_OK);

    fc_bloom_destroy(filter);
}

/* Test clear operation */
TEST(test_bloom_clear) {
    fc_bloom_config_t config = {
        .expected_elements = 100,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    const char* item = "test";
    fc_status_t err = fc_bloom_add(filter, item, strlen(item));
    ASSERT_EQ(err, FC_OK);

    bool result;
    err = fc_bloom_contains(filter, item, strlen(item), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == true);

    /* Clear the filter */
    err = fc_bloom_clear(filter);
    ASSERT_EQ(err, FC_OK);

    /* Item should no longer be found */
    err = fc_bloom_contains(filter, item, strlen(item), &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == false);

    /* Stats should show zero elements */
    fc_bloom_stats_t stats;
    err = fc_bloom_get_stats(filter, &stats);
    ASSERT_EQ(err, FC_OK);
    ASSERT_EQ(stats.elements_added, 0);

    fc_bloom_destroy(filter);
}

/* Test invalid argument handling */
TEST(test_bloom_invalid_arguments) {
    fc_bloom_config_t config = {
        .expected_elements = 100,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    /* NULL filter */
    fc_status_t err = fc_bloom_add(NULL, "test", 4);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL data */
    err = fc_bloom_add(filter, NULL, 4);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL filter in contains */
    bool result;
    err = fc_bloom_contains(NULL, "test", 4, &result);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL data in contains */
    err = fc_bloom_contains(filter, NULL, 4, &result);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL result pointer */
    err = fc_bloom_contains(filter, "test", 4, NULL);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL filter in clear */
    err = fc_bloom_clear(NULL);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL filter in get_stats */
    fc_bloom_stats_t stats;
    err = fc_bloom_get_stats(NULL, &stats);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    /* NULL stats pointer */
    err = fc_bloom_get_stats(filter, NULL);
    ASSERT_EQ(err, FC_ERR_INVALID_ARG);

    fc_bloom_destroy(filter);
    fc_bloom_destroy(NULL);  /* Should not crash */
}

/* Test false positive rate */
TEST(test_bloom_false_positive_rate) {
    fc_bloom_config_t config = {
        .expected_elements = 1000,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    /* Add 1000 elements */
    for (int i = 0; i < 1000; i++) {
        fc_status_t err = fc_bloom_add(filter, &i, sizeof(i));
        ASSERT_EQ(err, FC_OK);
    }

    /* Test 10000 non-added elements */
    int false_positives = 0;
    int test_count = 10000;
    for (int i = 1000; i < 1000 + test_count; i++) {
        bool result;
        fc_status_t err = fc_bloom_contains(filter, &i, sizeof(i), &result);
        ASSERT_EQ(err, FC_OK);
        if (result) {
            false_positives++;
        }
    }

    double actual_fpp = (double)false_positives / (double)test_count;
    FC_TEST_ASSERT(actual_fpp < 0.02);

    /* Check estimated FPP */
    fc_bloom_stats_t stats;
    fc_status_t err = fc_bloom_get_stats(filter, &stats);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(stats.estimated_fpp > 0.0);
    FC_TEST_ASSERT(stats.estimated_fpp < 0.02);

    fc_bloom_destroy(filter);
}

/* Test empty data handling */
TEST(test_bloom_empty_data) {
    fc_bloom_config_t config = {
        .expected_elements = 100,
        .false_positive_rate = 0.01
    };

    fc_bloom_filter_t* filter = fc_bloom_create(&config);
    FC_TEST_ASSERT(filter != NULL);

    /* Add empty data (length 0) */
    const char* empty = "";
    fc_status_t err = fc_bloom_add(filter, empty, 0);
    ASSERT_EQ(err, FC_OK);

    /* Check if empty data is found */
    bool result;
    err = fc_bloom_contains(filter, empty, 0, &result);
    ASSERT_EQ(err, FC_OK);
    FC_TEST_ASSERT(result == true);

    fc_bloom_destroy(filter);
}

/*
 * Test Suite Registration
 */
void register_bloom_filter_tests(void) {
    RUN_TEST(test_bloom_optimal_calculations);
    RUN_TEST(test_bloom_create_destroy);
    RUN_TEST(test_bloom_create_explicit);
    RUN_TEST(test_bloom_create_invalid);
    RUN_TEST(test_bloom_add_contains_basic);
    RUN_TEST(test_bloom_add_contains_numbers);
    RUN_TEST(test_bloom_batch_operations);
    RUN_TEST(test_bloom_clear);
    RUN_TEST(test_bloom_invalid_arguments);
    RUN_TEST(test_bloom_false_positive_rate);
    RUN_TEST(test_bloom_empty_data);
}
