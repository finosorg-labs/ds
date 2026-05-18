/**
 * @file test_roaring_bitmap.c
 * @brief Unit tests for Roaring Bitmap
 */

#include "test_framework.h"
#include <roaring_bitmap.h>
#include <platform.h>
#include <error.h>
#include <stdio.h>
#include <string.h>

/* Test basic create and destroy */
TEST(test_roaring_create_destroy) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);
    FC_TEST_ASSERT(fc_roaring_is_empty(bitmap));
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 0);
    fc_roaring_destroy(bitmap);
}

/* Test create from array */
TEST(test_roaring_create_from_array) {
    uint32_t values[] = {1, 5, 10, 100, 1000, 10000};
    size_t count = sizeof(values) / sizeof(values[0]);

    fc_roaring_bitmap_t* bitmap = fc_roaring_create_from_array(values, count);
    FC_TEST_ASSERT(bitmap != NULL);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), count);

    bool result;
    for (size_t i = 0; i < count; i++) {
        fc_status_t err = fc_roaring_contains(bitmap, values[i], &result);
        FC_TEST_ASSERT(err == FC_OK);
        FC_TEST_ASSERT(result);
    }

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    bitmap = fc_roaring_create_from_array(NULL, 10);
    FC_TEST_ASSERT(bitmap == NULL);

    bitmap = fc_roaring_create_from_array(values, 0);
    FC_TEST_ASSERT(bitmap == NULL);
}

/* Test create from range */
TEST(test_roaring_create_from_range) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create_from_range(100, 200);
    FC_TEST_ASSERT(bitmap != NULL);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 100);

    bool result;
    fc_roaring_contains(bitmap, 99, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_contains(bitmap, 100, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 150, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 199, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 200, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);

    /* Invalid range */
    bitmap = fc_roaring_create_from_range(200, 100);
    FC_TEST_ASSERT(bitmap == NULL);

    bitmap = fc_roaring_create_from_range(100, 100);
    FC_TEST_ASSERT(bitmap == NULL);
}

/* Test add single value */
TEST(test_roaring_add_single) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    fc_status_t err = fc_roaring_add(bitmap, 42);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 1);

    bool result;
    err = fc_roaring_contains(bitmap, 42, &result);
    FC_TEST_ASSERT(err == FC_OK);
    FC_TEST_ASSERT(result);

    err = fc_roaring_contains(bitmap, 43, &result);
    FC_TEST_ASSERT(err == FC_OK);
    FC_TEST_ASSERT(!result);

    /* Add duplicate */
    err = fc_roaring_add(bitmap, 42);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 1);

    fc_roaring_destroy(bitmap);

    /* Invalid input */
    err = fc_roaring_add(NULL, 42);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
}

/* Test add multiple values */
TEST(test_roaring_add_multiple) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 1000; i++) {
        fc_status_t err = fc_roaring_add(bitmap, i);
        FC_TEST_ASSERT(err == FC_OK);
    }

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 1000);

    bool result;
    for (uint32_t i = 0; i < 1000; i++) {
        fc_roaring_contains(bitmap, i, &result);
        FC_TEST_ASSERT(result);
    }

    fc_roaring_contains(bitmap, 1000, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);
}

/* Test add batch */
TEST(test_roaring_add_batch) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    uint32_t values[100];
    for (uint32_t i = 0; i < 100; i++) {
        values[i] = i * 10;
    }

    fc_status_t err = fc_roaring_add_batch(bitmap, values, 100);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 100);

    bool result;
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_contains(bitmap, values[i], &result);
        FC_TEST_ASSERT(result);
    }

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_add_batch(NULL, values, 100);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_add_batch(bitmap, NULL, 100);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test add range */
TEST(test_roaring_add_range) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    fc_status_t err = fc_roaring_add_range(bitmap, 1000, 2000);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 1000);

    bool result;
    fc_roaring_contains(bitmap, 999, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_contains(bitmap, 1000, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 1500, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 1999, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 2000, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_add_range(NULL, 1000, 2000);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_add_range(bitmap, 2000, 1000);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test contains batch */
TEST(test_roaring_contains_batch) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 100; i += 2) {
        fc_roaring_add(bitmap, i);
    }

    uint32_t values[100];
    bool results[100];
    for (uint32_t i = 0; i < 100; i++) {
        values[i] = i;
    }

    fc_status_t err = fc_roaring_contains_batch(bitmap, values, 100, results);
    FC_TEST_ASSERT(err == FC_OK);

    for (uint32_t i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            FC_TEST_ASSERT(results[i]);
        } else {
            FC_TEST_ASSERT(!results[i]);
        }
    }

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_contains_batch(NULL, values, 100, results);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_contains_batch(bitmap, NULL, 100, results);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    err = fc_roaring_contains_batch(bitmap, values, 100, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test clear */
TEST(test_roaring_clear) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(bitmap, i);
    }
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 100);

    fc_status_t err = fc_roaring_clear(bitmap);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 0);
    FC_TEST_ASSERT(fc_roaring_is_empty(bitmap));

    fc_roaring_destroy(bitmap);

    /* Invalid input */
    err = fc_roaring_clear(NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
}

/* Test min and max */
TEST(test_roaring_min_max) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    fc_roaring_add(bitmap, 10);
    fc_roaring_add(bitmap, 50);
    fc_roaring_add(bitmap, 100);

    uint32_t min_val, max_val;
    fc_status_t err = fc_roaring_min(bitmap, &min_val);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(min_val, 10);

    err = fc_roaring_max(bitmap, &max_val);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(max_val, 100);

    fc_roaring_destroy(bitmap);

    /* Empty bitmap */
    bitmap = fc_roaring_create();
    err = fc_roaring_min(bitmap, &min_val);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    err = fc_roaring_max(bitmap, &max_val);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_min(NULL, &min_val);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    fc_roaring_add(bitmap, 42);
    err = fc_roaring_min(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test to_array */
TEST(test_roaring_to_array) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    uint32_t input[] = {5, 10, 15, 20, 25};
    size_t count = sizeof(input) / sizeof(input[0]);

    for (size_t i = 0; i < count; i++) {
        fc_roaring_add(bitmap, input[i]);
    }

    uint32_t output[10];
    fc_status_t err = fc_roaring_to_array(bitmap, output);
    FC_TEST_ASSERT(err == FC_OK);

    for (size_t i = 0; i < count; i++) {
        ASSERT_EQ(output[i], input[i]);
    }

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_to_array(NULL, output);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_to_array(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test statistics */
TEST(test_roaring_stats) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 1000; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_roaring_stats_t stats;
    fc_status_t err = fc_roaring_get_stats(bitmap, &stats);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(stats.cardinality, 1000);
    FC_TEST_ASSERT(stats.num_containers > 0);
    FC_TEST_ASSERT(stats.memory_bytes > 0);

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_get_stats(NULL, &stats);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_get_stats(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test array to bitmap container conversion */
TEST(test_roaring_container_conversion) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    /* Add enough values to trigger array->bitmap conversion */
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(bitmap, i);
    }

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 5000);

    bool result;
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_contains(bitmap, i, &result);
        FC_TEST_ASSERT(result);
    }

    fc_roaring_stats_t stats;
    fc_roaring_get_stats(bitmap, &stats);
    FC_TEST_ASSERT(stats.num_bitmap_containers > 0);

    fc_roaring_destroy(bitmap);
}

/* Test multiple containers (different high 16 bits) */
TEST(test_roaring_multiple_containers) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    /* Add values in different containers */
    fc_roaring_add(bitmap, 100);           /* Container 0 */
    fc_roaring_add(bitmap, 65536 + 200);   /* Container 1 */
    fc_roaring_add(bitmap, 131072 + 300);  /* Container 2 */

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 3);

    bool result;
    fc_roaring_contains(bitmap, 100, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 65536 + 200, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 131072 + 300, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 101, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_stats_t stats;
    fc_roaring_get_stats(bitmap, &stats);
    ASSERT_EQ(stats.num_containers, 3);

    fc_roaring_destroy(bitmap);
}

/* Test optimize */
TEST(test_roaring_optimize) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_status_t err = fc_roaring_optimize(bitmap);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 100);

    fc_roaring_destroy(bitmap);

    /* Invalid input */
    err = fc_roaring_optimize(NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
}

/* Test equals */
TEST(test_roaring_equals) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();
    FC_TEST_ASSERT(a != NULL && b != NULL);

    /* Empty bitmaps are equal */
    FC_TEST_ASSERT(fc_roaring_equals(a, b));

    /* Add same values */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
        fc_roaring_add(b, i);
    }
    FC_TEST_ASSERT(fc_roaring_equals(a, b));

    /* Add different value to one */
    fc_roaring_add(a, 1000);
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);

    /* Invalid inputs */
    FC_TEST_ASSERT(!fc_roaring_equals(NULL, NULL));
}

/* Test cardinality edge cases */
TEST(test_roaring_cardinality_edge_cases) {
    /* NULL bitmap */
    ASSERT_EQ(fc_roaring_cardinality(NULL), 0);

    /* Empty bitmap */
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 0);
    FC_TEST_ASSERT(fc_roaring_is_empty(bitmap));

    /* Single element */
    fc_roaring_add(bitmap, 42);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 1);
    FC_TEST_ASSERT(!fc_roaring_is_empty(bitmap));

    fc_roaring_destroy(bitmap);
}

/* Test is_empty */
TEST(test_roaring_is_empty) {
    FC_TEST_ASSERT(fc_roaring_is_empty(NULL));

    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(fc_roaring_is_empty(bitmap));

    fc_roaring_add(bitmap, 1);
    FC_TEST_ASSERT(!fc_roaring_is_empty(bitmap));

    fc_roaring_clear(bitmap);
    FC_TEST_ASSERT(fc_roaring_is_empty(bitmap));

    fc_roaring_destroy(bitmap);
}

void register_roaring_bitmap_tests(void) {
    RUN_TEST(test_roaring_create_destroy);
    RUN_TEST(test_roaring_create_from_array);
    RUN_TEST(test_roaring_create_from_range);
    RUN_TEST(test_roaring_add_single);
    RUN_TEST(test_roaring_add_multiple);
    RUN_TEST(test_roaring_add_batch);
    RUN_TEST(test_roaring_add_range);
    RUN_TEST(test_roaring_contains_batch);
    RUN_TEST(test_roaring_clear);
    RUN_TEST(test_roaring_min_max);
    RUN_TEST(test_roaring_to_array);
    RUN_TEST(test_roaring_stats);
    RUN_TEST(test_roaring_container_conversion);
    RUN_TEST(test_roaring_multiple_containers);
    RUN_TEST(test_roaring_optimize);
    RUN_TEST(test_roaring_equals);
    RUN_TEST(test_roaring_cardinality_edge_cases);
    RUN_TEST(test_roaring_is_empty);
}
