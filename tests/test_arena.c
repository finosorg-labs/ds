/**
 * @file test_arena.c
 * @brief Unit tests for arena allocator
 */

#include "arena.h"
#include "test_framework.h"
#include <string.h>

TEST(test_arena_create_destroy) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);
    ASSERT_EQ(fc_arena_capacity(arena), 1024);
    ASSERT_EQ(fc_arena_used(arena), 0);
    ASSERT_EQ(fc_arena_available(arena), 1024);
    fc_arena_destroy(arena);
}

TEST(test_arena_create_invalid) {
    fc_arena_t* arena = fc_arena_create(0);
    ASSERT_NULL(arena);
}

TEST(test_arena_basic_alloc) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    void* ptr1 = fc_arena_alloc(arena, 64);
    ASSERT_NOT_NULL(ptr1);
    FC_TEST_ASSERT(fc_arena_used(arena) > 0);

    void* ptr2 = fc_arena_alloc(arena, 128);
    ASSERT_NOT_NULL(ptr2);
    FC_TEST_ASSERT(ptr2 > ptr1);

    size_t used = fc_arena_used(arena);
    FC_TEST_ASSERT(used <= 1024);

    fc_arena_destroy(arena);
}

TEST(test_arena_alloc_aligned) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    void* ptr16 = fc_arena_alloc_aligned(arena, 10, 16);
    ASSERT_NOT_NULL(ptr16);
    FC_TEST_ASSERT(((uintptr_t)ptr16 & 15) == 0);

    void* ptr32 = fc_arena_alloc_aligned(arena, 10, 32);
    ASSERT_NOT_NULL(ptr32);
    FC_TEST_ASSERT(((uintptr_t)ptr32 & 31) == 0);

    void* ptr64 = fc_arena_alloc_aligned(arena, 10, 64);
    ASSERT_NOT_NULL(ptr64);
    FC_TEST_ASSERT(((uintptr_t)ptr64 & 63) == 0);

    fc_arena_destroy(arena);
}

TEST(test_arena_alloc_invalid_alignment) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    void* ptr = fc_arena_alloc_aligned(arena, 10, 3);
    ASSERT_NULL(ptr);

    ptr = fc_arena_alloc_aligned(arena, 10, 7);
    ASSERT_NULL(ptr);

    fc_arena_destroy(arena);
}

TEST(test_arena_exhaustion) {
    fc_arena_t* arena = fc_arena_create(128);
    ASSERT_NOT_NULL(arena);

    void* ptr1 = fc_arena_alloc(arena, 64);
    ASSERT_NOT_NULL(ptr1);

    void* ptr2 = fc_arena_alloc(arena, 64);
    ASSERT_NOT_NULL(ptr2);

    void* ptr3 = fc_arena_alloc(arena, 64);
    ASSERT_NULL(ptr3);

    FC_TEST_ASSERT(fc_arena_available(arena) < 64);

    fc_arena_destroy(arena);
}

TEST(test_arena_reset) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    void* ptr1 = fc_arena_alloc(arena, 256);
    ASSERT_NOT_NULL(ptr1);
    size_t used_before = fc_arena_used(arena);
    FC_TEST_ASSERT(used_before > 0);

    fc_arena_reset(arena);
    ASSERT_EQ(fc_arena_used(arena), 0);
    ASSERT_EQ(fc_arena_available(arena), 1024);

    void* ptr2 = fc_arena_alloc(arena, 256);
    ASSERT_NOT_NULL(ptr2);

    fc_arena_destroy(arena);
}

TEST(test_arena_write_read) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    int* numbers = (int*)fc_arena_alloc(arena, sizeof(int) * 10);
    ASSERT_NOT_NULL(numbers);

    for (int i = 0; i < 10; i++) {
        numbers[i] = i * 10;
    }

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(numbers[i], i * 10);
    }

    fc_arena_destroy(arena);
}

TEST(test_arena_multiple_types) {
    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NOT_NULL(arena);

    int* int_ptr = (int*)fc_arena_alloc(arena, sizeof(int));
    ASSERT_NOT_NULL(int_ptr);
    *int_ptr = 42;

    double* double_ptr = (double*)fc_arena_alloc(arena, sizeof(double));
    ASSERT_NOT_NULL(double_ptr);
    *double_ptr = 3.14159;

    char* str = (char*)fc_arena_alloc(arena, 20);
    ASSERT_NOT_NULL(str);
    strcpy(str, "Hello, Arena!");

    ASSERT_EQ(*int_ptr, 42);
    FC_TEST_ASSERT(*double_ptr == 3.14159);
    FC_TEST_ASSERT(strcmp(str, "Hello, Arena!") == 0);

    fc_arena_destroy(arena);
}

TEST(test_arena_null_checks) {
    ASSERT_NULL(fc_arena_alloc(NULL, 10));
    ASSERT_NULL(fc_arena_alloc_aligned(NULL, 10, 8));

    fc_arena_t* arena = fc_arena_create(1024);
    ASSERT_NULL(fc_arena_alloc(arena, 0));
    ASSERT_NULL(fc_arena_alloc_aligned(arena, 0, 8));

    fc_arena_reset(NULL);
    fc_arena_destroy(NULL);

    ASSERT_EQ(fc_arena_used(NULL), 0);
    ASSERT_EQ(fc_arena_capacity(NULL), 0);
    ASSERT_EQ(fc_arena_available(NULL), 0);

    fc_arena_destroy(arena);
}

TEST(test_arena_large_allocation) {
    fc_arena_t* arena = fc_arena_create(1024 * 1024);
    ASSERT_NOT_NULL(arena);

    void* large = fc_arena_alloc(arena, 512 * 1024);
    ASSERT_NOT_NULL(large);

    size_t used = fc_arena_used(arena);
    FC_TEST_ASSERT(used >= 512 * 1024);

    fc_arena_destroy(arena);
}

void register_arena_tests(void) {
    RUN_TEST(test_arena_create_destroy);
    RUN_TEST(test_arena_create_invalid);
    RUN_TEST(test_arena_basic_alloc);
    RUN_TEST(test_arena_alloc_aligned);
    RUN_TEST(test_arena_alloc_invalid_alignment);
    RUN_TEST(test_arena_exhaustion);
    RUN_TEST(test_arena_reset);
    RUN_TEST(test_arena_write_read);
    RUN_TEST(test_arena_multiple_types);
    RUN_TEST(test_arena_null_checks);
    RUN_TEST(test_arena_large_allocation);
}
