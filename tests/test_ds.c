/**
 * @file test_ds.c
 * @brief ds module test entry point
 *
 * This file serves as the main test registration point for the ds module.
 * Individual test modules are in separate files:
 */

#include "test_framework.h"

/* External test registration functions from sub-modules */
extern void register_bloom_filter_tests(void);
extern void register_roaring_bitmap_tests(void);
extern void register_ring_buffer_tests(void);
extern void register_ring_buffer_mt_tests(void);
extern void register_generic_ring_buffer_tests(void);
extern void register_mem_pool_tests(void);
extern void register_arena_tests(void);

/* Entry point for ds tests */
void register_ds_tests(void) {
    /* Register all sub-module tests */
    register_bloom_filter_tests();
    register_roaring_bitmap_tests();
    register_ring_buffer_tests();
    register_ring_buffer_mt_tests();
    register_generic_ring_buffer_tests();
    register_mem_pool_tests();
    register_arena_tests();
}
