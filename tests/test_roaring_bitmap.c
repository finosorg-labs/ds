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

/* Test clone */
TEST(test_roaring_clone) {
    /* Clone NULL bitmap */
    fc_roaring_bitmap_t* clone = fc_roaring_clone(NULL);
    FC_TEST_ASSERT(clone == NULL);

    /* Clone empty bitmap */
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    clone = fc_roaring_clone(bitmap);
    FC_TEST_ASSERT(clone != NULL);
    FC_TEST_ASSERT(fc_roaring_is_empty(clone));
    fc_roaring_destroy(clone);

    /* Clone bitmap with array container */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(bitmap, i);
    }
    clone = fc_roaring_clone(bitmap);
    FC_TEST_ASSERT(clone != NULL);
    ASSERT_EQ(fc_roaring_cardinality(clone), 100);
    FC_TEST_ASSERT(fc_roaring_equals(bitmap, clone));
    fc_roaring_destroy(clone);

    /* Clone bitmap with bitmap container */
    fc_roaring_clear(bitmap);
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(bitmap, i);
    }
    clone = fc_roaring_clone(bitmap);
    FC_TEST_ASSERT(clone != NULL);
    ASSERT_EQ(fc_roaring_cardinality(clone), 5000);
    FC_TEST_ASSERT(fc_roaring_equals(bitmap, clone));
    fc_roaring_destroy(clone);

    /* Clone bitmap with multiple containers */
    fc_roaring_clear(bitmap);
    fc_roaring_add(bitmap, 100);
    fc_roaring_add(bitmap, 65536 + 200);
    fc_roaring_add(bitmap, 131072 + 300);
    clone = fc_roaring_clone(bitmap);
    FC_TEST_ASSERT(clone != NULL);
    ASSERT_EQ(fc_roaring_cardinality(clone), 3);
    FC_TEST_ASSERT(fc_roaring_equals(bitmap, clone));
    fc_roaring_destroy(clone);

    fc_roaring_destroy(bitmap);
}

/* Test union */
TEST(test_roaring_union) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 50; i < 150; i++) {
        fc_roaring_add(b, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_union(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 150);

    bool contains;
    fc_roaring_contains(result, 0, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 149, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 150, &contains);
    FC_TEST_ASSERT(!contains);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);

    /* Invalid inputs */
    result = fc_roaring_union(NULL, NULL);
    FC_TEST_ASSERT(result == NULL);
}

/* Test intersection */
TEST(test_roaring_intersection) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 50; i < 150; i++) {
        fc_roaring_add(b, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_intersection(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 50);

    bool contains;
    fc_roaring_contains(result, 49, &contains);
    FC_TEST_ASSERT(!contains);
    fc_roaring_contains(result, 50, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 99, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 100, &contains);
    FC_TEST_ASSERT(!contains);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);

    /* Invalid inputs */
    result = fc_roaring_intersection(NULL, NULL);
    FC_TEST_ASSERT(result == NULL);
}

/* Test difference */
TEST(test_roaring_difference) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 50; i < 150; i++) {
        fc_roaring_add(b, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_difference(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 50);

    bool contains;
    fc_roaring_contains(result, 0, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 49, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 50, &contains);
    FC_TEST_ASSERT(!contains);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);

    /* Invalid inputs */
    result = fc_roaring_difference(NULL, NULL);
    FC_TEST_ASSERT(result == NULL);
}

/* Test xor */
TEST(test_roaring_xor) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 50; i < 150; i++) {
        fc_roaring_add(b, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_xor(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 100);

    bool contains;
    fc_roaring_contains(result, 0, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 49, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 50, &contains);
    FC_TEST_ASSERT(!contains);
    fc_roaring_contains(result, 99, &contains);
    FC_TEST_ASSERT(!contains);
    fc_roaring_contains(result, 100, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(result, 149, &contains);
    FC_TEST_ASSERT(contains);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);

    /* Invalid inputs */
    result = fc_roaring_xor(NULL, NULL);
    FC_TEST_ASSERT(result == NULL);
}

/* Test remove */
TEST(test_roaring_remove) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_status_t err = fc_roaring_remove(bitmap, 50);
    FC_TEST_ASSERT(err == FC_OK);

    fc_roaring_destroy(bitmap);

    /* Invalid input */
    err = fc_roaring_remove(NULL, 50);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
}

/* Test remove_range */
TEST(test_roaring_remove_range) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_status_t err = fc_roaring_remove_range(bitmap, 25, 75);
    FC_TEST_ASSERT(err == FC_OK);

    fc_roaring_destroy(bitmap);

    /* Invalid inputs */
    err = fc_roaring_remove_range(NULL, 25, 75);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    bitmap = fc_roaring_create();
    err = fc_roaring_remove_range(bitmap, 75, 25);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);
    fc_roaring_destroy(bitmap);
}

/* Test container insertion with memmove */
TEST(test_roaring_container_insertion) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    /* Add values in reverse order to trigger memmove during insertion */
    fc_roaring_add(bitmap, 200000);  /* Container 3 */
    fc_roaring_add(bitmap, 150000);  /* Container 2 */
    fc_roaring_add(bitmap, 100000);  /* Container 1 */
    fc_roaring_add(bitmap, 50000);   /* Container 0 */

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 4);

    bool contains;
    fc_roaring_contains(bitmap, 50000, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(bitmap, 100000, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(bitmap, 150000, &contains);
    FC_TEST_ASSERT(contains);
    fc_roaring_contains(bitmap, 200000, &contains);
    FC_TEST_ASSERT(contains);

    fc_roaring_destroy(bitmap);
}

/* Test array container with insertion in middle */
TEST(test_roaring_array_insertion) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    /* Add values that will trigger insertion in middle of array */
    fc_roaring_add(bitmap, 10);
    fc_roaring_add(bitmap, 30);
    fc_roaring_add(bitmap, 50);
    fc_roaring_add(bitmap, 20);  /* Insert in middle */
    fc_roaring_add(bitmap, 40);  /* Insert in middle */

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 5);

    /* Verify all values are present */
    bool contains;
    for (uint32_t val = 10; val <= 50; val += 10) {
        fc_roaring_contains(bitmap, val, &contains);
        FC_TEST_ASSERT(contains);
    }

    fc_roaring_destroy(bitmap);
}

/* Test capacity expansion */
TEST(test_roaring_capacity_expansion) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    /* Add many containers to trigger capacity expansion */
    for (uint32_t i = 0; i < 10; i++) {
        fc_roaring_add(bitmap, i * 65536);
    }

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 10);

    fc_roaring_stats_t stats;
    fc_roaring_get_stats(bitmap, &stats);
    ASSERT_EQ(stats.num_containers, 10);

    fc_roaring_destroy(bitmap);
}

/* Test destroy with NULL */
TEST(test_roaring_destroy_null) {
    fc_roaring_destroy(NULL);  /* Should not crash */
}

/* Test set operations with bitmap containers */
TEST(test_roaring_set_ops_bitmap_containers) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* Add enough values to create bitmap containers */
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 2500; i < 7500; i++) {
        fc_roaring_add(b, i);
    }

    /* Test union with bitmap containers */
    fc_roaring_bitmap_t* union_result = fc_roaring_union(a, b);
    FC_TEST_ASSERT(union_result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(union_result), 7500);
    fc_roaring_destroy(union_result);

    /* Test intersection with bitmap containers */
    fc_roaring_bitmap_t* inter_result = fc_roaring_intersection(a, b);
    FC_TEST_ASSERT(inter_result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(inter_result), 2500);
    fc_roaring_destroy(inter_result);

    /* Test difference with bitmap containers */
    fc_roaring_bitmap_t* diff_result = fc_roaring_difference(a, b);
    FC_TEST_ASSERT(diff_result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(diff_result), 2500);
    fc_roaring_destroy(diff_result);

    /* Test xor with bitmap containers */
    fc_roaring_bitmap_t* xor_result = fc_roaring_xor(a, b);
    FC_TEST_ASSERT(xor_result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(xor_result), 5000);
    fc_roaring_destroy(xor_result);

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test set operations with no overlap */
TEST(test_roaring_set_ops_no_overlap) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 200; i < 300; i++) {
        fc_roaring_add(b, i);
    }

    /* Intersection should be empty */
    fc_roaring_bitmap_t* inter = fc_roaring_intersection(a, b);
    FC_TEST_ASSERT(inter != NULL);
    ASSERT_EQ(fc_roaring_cardinality(inter), 0);
    fc_roaring_destroy(inter);

    /* Union should have all elements */
    fc_roaring_bitmap_t* union_result = fc_roaring_union(a, b);
    FC_TEST_ASSERT(union_result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(union_result), 200);
    fc_roaring_destroy(union_result);

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test contains with non-existent container */
TEST(test_roaring_contains_missing_container) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    fc_roaring_add(bitmap, 100);
    fc_roaring_add(bitmap, 200);

    bool result;
    /* Query value in a different container that doesn't exist */
    fc_status_t err = fc_roaring_contains(bitmap, 65536 + 100, &result);
    FC_TEST_ASSERT(err == FC_OK);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);
}

/* Test create_from_array with NULL */
TEST(test_roaring_create_from_array_null) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create_from_array(NULL, 0);
    FC_TEST_ASSERT(bitmap == NULL);
}

/* Test create_from_range edge cases */
TEST(test_roaring_create_from_range_edge) {
    /* Same start and end */
    fc_roaring_bitmap_t* bitmap = fc_roaring_create_from_range(100, 100);
    FC_TEST_ASSERT(bitmap == NULL);

    /* Inverted range */
    bitmap = fc_roaring_create_from_range(200, 100);
    FC_TEST_ASSERT(bitmap == NULL);
}

/* Test add_batch with empty array */
TEST(test_roaring_add_batch_empty) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    uint32_t values[1];
    fc_status_t err = fc_roaring_add_batch(bitmap, values, 0);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 0);

    fc_roaring_destroy(bitmap);
}

/* Test add_range with invalid range */
TEST(test_roaring_add_range_invalid) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    FC_TEST_ASSERT(bitmap != NULL);

    fc_status_t err = fc_roaring_add_range(bitmap, 100, 100);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    err = fc_roaring_add_range(bitmap, 200, 100);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    fc_roaring_destroy(bitmap);
}

/* Test min/max with NULL output */
TEST(test_roaring_min_max_null_output) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    fc_roaring_add(bitmap, 42);

    fc_status_t err = fc_roaring_min(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    err = fc_roaring_max(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    fc_roaring_destroy(bitmap);
}

/* Test to_array with NULL */
TEST(test_roaring_to_array_null) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    fc_roaring_add(bitmap, 42);

    fc_status_t err = fc_roaring_to_array(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    fc_roaring_destroy(bitmap);
}

/* Test get_stats with NULL */
TEST(test_roaring_get_stats_null) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    fc_roaring_add(bitmap, 42);

    fc_status_t err = fc_roaring_get_stats(bitmap, NULL);
    FC_TEST_ASSERT(err == FC_ERR_INVALID_ARG);

    fc_roaring_destroy(bitmap);
}

/* Test equals with NULL */
TEST(test_roaring_equals_null) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    FC_TEST_ASSERT(!fc_roaring_equals(NULL, bitmap));
    FC_TEST_ASSERT(!fc_roaring_equals(bitmap, NULL));
    FC_TEST_ASSERT(!fc_roaring_equals(NULL, NULL));

    fc_roaring_destroy(bitmap);
}

/* Test clone with bitmap container */
TEST(test_roaring_clone_bitmap_container) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Create bitmap container */
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(bitmap, i);
    }

    fc_roaring_bitmap_t* clone = fc_roaring_clone(bitmap);
    FC_TEST_ASSERT(clone != NULL);
    ASSERT_EQ(fc_roaring_cardinality(clone), 5000);
    FC_TEST_ASSERT(fc_roaring_equals(bitmap, clone));

    fc_roaring_destroy(clone);
    fc_roaring_destroy(bitmap);
}

/* Test binary search in find_container */
TEST(test_roaring_find_container_search) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Add values in multiple containers to test binary search */
    fc_roaring_add(bitmap, 0);           /* Container 0 */
    fc_roaring_add(bitmap, 65536);       /* Container 1 */
    fc_roaring_add(bitmap, 131072);      /* Container 2 */
    fc_roaring_add(bitmap, 196608);      /* Container 3 */
    fc_roaring_add(bitmap, 262144);      /* Container 4 */

    /* Test contains for values in different containers */
    bool result;
    fc_roaring_contains(bitmap, 0, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 131072, &result);
    FC_TEST_ASSERT(result);

    fc_roaring_contains(bitmap, 262144, &result);
    FC_TEST_ASSERT(result);

    /* Test contains for non-existent values */
    fc_roaring_contains(bitmap, 1, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_contains(bitmap, 65537, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);
}

/* Test binary search with many containers */
TEST(test_roaring_binary_search_many_containers) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Add 20 containers to test binary search thoroughly */
    for (uint32_t i = 0; i < 20; i++) {
        fc_roaring_add(bitmap, i * 65536);
    }

    /* Test finding containers at different positions */
    bool result;

    /* First container */
    fc_roaring_contains(bitmap, 0, &result);
    FC_TEST_ASSERT(result);

    /* Middle container */
    fc_roaring_contains(bitmap, 10 * 65536, &result);
    FC_TEST_ASSERT(result);

    /* Last container */
    fc_roaring_contains(bitmap, 19 * 65536, &result);
    FC_TEST_ASSERT(result);

    /* Non-existent container in the middle */
    fc_roaring_contains(bitmap, 25 * 65536, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);
}

/* Test container_destroy with NULL */
TEST(test_roaring_container_destroy_null) {
    /* This tests the NULL check in container_destroy */
    /* We can't call it directly, but we can test via fc_roaring_destroy */
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();
    fc_roaring_destroy(bitmap);  /* Should handle empty containers gracefully */
}

/* Test equals with different cardinalities */
TEST(test_roaring_equals_different_cardinality) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    fc_roaring_add(a, 1);
    fc_roaring_add(a, 2);

    fc_roaring_add(b, 1);

    /* Different cardinalities should not be equal */
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test equals with different keys */
TEST(test_roaring_equals_different_keys) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    fc_roaring_add(a, 100);
    fc_roaring_add(b, 65636);  /* Different container */

    /* Different keys should not be equal */
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test equals with different container types */
TEST(test_roaring_equals_different_types) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* a has array container */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    /* b has bitmap container */
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(b, i);
    }

    /* Different types should not be equal */
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test equals with same type but different values */
TEST(test_roaring_equals_same_type_different_values) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* Both have array containers with same cardinality */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 100; i < 200; i++) {
        fc_roaring_add(b, i);
    }

    /* Same cardinality but different values */
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test add_range with large range */
TEST(test_roaring_add_range_large) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Add a large range spanning multiple containers */
    fc_status_t err = fc_roaring_add_range(bitmap, 0, 100000);
    FC_TEST_ASSERT(err == FC_OK);
    ASSERT_EQ(fc_roaring_cardinality(bitmap), 100000);

    fc_roaring_destroy(bitmap);
}

/* Test union with empty bitmaps */
TEST(test_roaring_union_empty) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    fc_roaring_bitmap_t* result = fc_roaring_union(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 0);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test intersection with empty bitmap */
TEST(test_roaring_intersection_empty) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_intersection(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 0);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test difference with empty bitmap */
TEST(test_roaring_difference_empty) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_difference(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 100);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test xor with empty bitmap */
TEST(test_roaring_xor_empty) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    fc_roaring_bitmap_t* result = fc_roaring_xor(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 100);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test array container at capacity limit */
TEST(test_roaring_array_container_full) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Add exactly ARRAY_CONTAINER_MAX_SIZE elements to fill array container */
    for (uint32_t i = 0; i < 4096; i++) {
        fc_roaring_add(bitmap, i);
    }

    ASSERT_EQ(fc_roaring_cardinality(bitmap), 4096);

    /* Verify all elements are present */
    bool result;
    for (uint32_t i = 0; i < 4096; i++) {
        fc_roaring_contains(bitmap, i, &result);
        FC_TEST_ASSERT(result);
    }

    fc_roaring_destroy(bitmap);
}

/* Test binary search not finding a key */
TEST(test_roaring_binary_search_not_found) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    /* Add containers with gaps */
    fc_roaring_add(bitmap, 0);           /* Container 0 */
    fc_roaring_add(bitmap, 131072);      /* Container 2 */
    fc_roaring_add(bitmap, 262144);      /* Container 4 */

    /* Try to find value in non-existent container 1 */
    bool result;
    fc_roaring_contains(bitmap, 65536 + 100, &result);
    FC_TEST_ASSERT(!result);

    /* Try to find value in non-existent container 3 */
    fc_roaring_contains(bitmap, 196608 + 100, &result);
    FC_TEST_ASSERT(!result);

    /* Try to find value beyond all containers */
    fc_roaring_contains(bitmap, 500000, &result);
    FC_TEST_ASSERT(!result);

    fc_roaring_destroy(bitmap);
}

/* Test intersection with no matching containers */
TEST(test_roaring_intersection_no_matching_containers) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* a has container 0 */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    /* b has container 1 */
    for (uint32_t i = 65536; i < 65636; i++) {
        fc_roaring_add(b, i);
    }

    /* No matching containers, intersection should be empty */
    fc_roaring_bitmap_t* result = fc_roaring_intersection(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 0);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test difference with no matching containers */
TEST(test_roaring_difference_no_matching_containers) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* a has container 0 */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    /* b has container 1 */
    for (uint32_t i = 65536; i < 65636; i++) {
        fc_roaring_add(b, i);
    }

    /* No matching containers, difference should be all of a */
    fc_roaring_bitmap_t* result = fc_roaring_difference(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 100);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test xor with no matching containers */
TEST(test_roaring_xor_no_matching_containers) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* a has container 0 */
    for (uint32_t i = 0; i < 100; i++) {
        fc_roaring_add(a, i);
    }

    /* b has container 1 */
    for (uint32_t i = 65536; i < 65636; i++) {
        fc_roaring_add(b, i);
    }

    /* No matching containers, xor should have all elements */
    fc_roaring_bitmap_t* result = fc_roaring_xor(a, b);
    FC_TEST_ASSERT(result != NULL);
    ASSERT_EQ(fc_roaring_cardinality(result), 200);

    fc_roaring_destroy(result);
    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test equals with bitmap containers having different values */
TEST(test_roaring_equals_bitmap_different_values) {
    fc_roaring_bitmap_t* a = fc_roaring_create();
    fc_roaring_bitmap_t* b = fc_roaring_create();

    /* Both have bitmap containers */
    for (uint32_t i = 0; i < 5000; i++) {
        fc_roaring_add(a, i);
    }
    for (uint32_t i = 1000; i < 6000; i++) {
        fc_roaring_add(b, i);
    }

    /* Different values in bitmap containers */
    FC_TEST_ASSERT(!fc_roaring_equals(a, b));

    fc_roaring_destroy(a);
    fc_roaring_destroy(b);
}

/* Test contains on empty bitmap */
TEST(test_roaring_contains_empty_bitmap) {
    fc_roaring_bitmap_t* bitmap = fc_roaring_create();

    bool result;
    fc_status_t err = fc_roaring_contains(bitmap, 42, &result);
    FC_TEST_ASSERT(err == FC_OK);
    FC_TEST_ASSERT(!result);

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
    RUN_TEST(test_roaring_clone);
    RUN_TEST(test_roaring_union);
    RUN_TEST(test_roaring_intersection);
    RUN_TEST(test_roaring_difference);
    RUN_TEST(test_roaring_xor);
    RUN_TEST(test_roaring_remove);
    RUN_TEST(test_roaring_remove_range);
    RUN_TEST(test_roaring_container_insertion);
    RUN_TEST(test_roaring_array_insertion);
    RUN_TEST(test_roaring_capacity_expansion);
    RUN_TEST(test_roaring_destroy_null);
    RUN_TEST(test_roaring_set_ops_bitmap_containers);
    RUN_TEST(test_roaring_set_ops_no_overlap);
    RUN_TEST(test_roaring_contains_missing_container);
    RUN_TEST(test_roaring_create_from_array_null);
    RUN_TEST(test_roaring_create_from_range_edge);
    RUN_TEST(test_roaring_add_batch_empty);
    RUN_TEST(test_roaring_add_range_invalid);
    RUN_TEST(test_roaring_min_max_null_output);
    RUN_TEST(test_roaring_to_array_null);
    RUN_TEST(test_roaring_get_stats_null);
    RUN_TEST(test_roaring_equals_null);
    RUN_TEST(test_roaring_clone_bitmap_container);
    RUN_TEST(test_roaring_find_container_search);
    RUN_TEST(test_roaring_binary_search_many_containers);
    RUN_TEST(test_roaring_container_destroy_null);
    RUN_TEST(test_roaring_equals_different_cardinality);
    RUN_TEST(test_roaring_equals_different_keys);
    RUN_TEST(test_roaring_equals_different_types);
    RUN_TEST(test_roaring_equals_same_type_different_values);
    RUN_TEST(test_roaring_add_range_large);
    RUN_TEST(test_roaring_union_empty);
    RUN_TEST(test_roaring_intersection_empty);
    RUN_TEST(test_roaring_difference_empty);
    RUN_TEST(test_roaring_xor_empty);
    RUN_TEST(test_roaring_array_container_full);
    RUN_TEST(test_roaring_binary_search_not_found);
    RUN_TEST(test_roaring_intersection_no_matching_containers);
    RUN_TEST(test_roaring_difference_no_matching_containers);
    RUN_TEST(test_roaring_xor_no_matching_containers);
    RUN_TEST(test_roaring_equals_bitmap_different_values);
    RUN_TEST(test_roaring_contains_empty_bitmap);
}
