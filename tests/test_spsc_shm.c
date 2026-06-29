/**
 * @file test_spsc_shm.c
 * @brief Unit tests for SPSC shared memory operations
 */

#include "spsc_shm.h"
#include "test_framework.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

static void generate_unique_name(char* buf, size_t len) {
    snprintf(buf, len, "test_spsc_%ld_%d", (long)time(NULL), rand() % 10000);
}

TEST(test_spsc_shm_create_basic) {
    fc_spsc_shm_t shm;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&shm, name, 4096);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);
    ASSERT_NOT_NULL(fc_spsc_shm_base(&shm));
    ASSERT_EQ(fc_spsc_shm_size(&shm), 4096);

    fc_spsc_shm_close(&shm);
    fc_spsc_shm_unlink(name);
}

TEST(test_spsc_shm_create_invalid_args) {
    fc_spsc_shm_t shm;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(NULL, "test", 4096);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_INVALID_ARG);

    status = fc_spsc_shm_create(&shm, NULL, 4096);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_INVALID_ARG);

    status = fc_spsc_shm_create(&shm, "test", 0);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_INVALID_ARG);
}

TEST(test_spsc_shm_create_and_open) {
    fc_spsc_shm_t creator, opener;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 8192;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&creator, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);
    ASSERT_NOT_NULL(fc_spsc_shm_base(&creator));

    status = fc_spsc_shm_open(&opener, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);
    ASSERT_NOT_NULL(fc_spsc_shm_base(&opener));
    ASSERT_EQ(fc_spsc_shm_size(&opener), size);

    fc_spsc_shm_close(&opener);
    fc_spsc_shm_close(&creator);
    fc_spsc_shm_unlink(name);
}

TEST(test_spsc_shm_data_exchange) {
    fc_spsc_shm_t writer, reader;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 1024;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&writer, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    const char* test_data = "Hello from shared memory!";
    void* writer_base = fc_spsc_shm_base(&writer);
    memcpy(writer_base, test_data, strlen(test_data) + 1);

    status = fc_spsc_shm_open(&reader, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    void* reader_base = fc_spsc_shm_base(&reader);
    ASSERT_EQ(strcmp((const char*)reader_base, test_data), 0);

    fc_spsc_shm_close(&reader);
    fc_spsc_shm_close(&writer);
    fc_spsc_shm_unlink(name);
}

TEST(test_spsc_shm_large_segment) {
    fc_spsc_shm_t shm;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 1024 * 1024;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&shm, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);
    ASSERT_NOT_NULL(fc_spsc_shm_base(&shm));
    ASSERT_EQ(fc_spsc_shm_size(&shm), size);

    uint8_t* base = (uint8_t*)fc_spsc_shm_base(&shm);
    for (size_t i = 0; i < size; i += 4096) {
        base[i] = (uint8_t)(i & 0xFF);
    }

    for (size_t i = 0; i < size; i += 4096) {
        ASSERT_EQ(base[i], (uint8_t)(i & 0xFF));
    }

    fc_spsc_shm_close(&shm);
    fc_spsc_shm_unlink(name);
}

TEST(test_spsc_shm_open_nonexistent) {
    fc_spsc_shm_t shm;
    char name[256];
    generate_unique_name(name, sizeof(name));

    fc_spsc_shm_status_t status = fc_spsc_shm_open(&shm, name, 4096);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_OPEN);
}

#ifndef _WIN32
TEST(test_spsc_shm_size_mismatch) {
    fc_spsc_shm_t creator, opener;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 4096;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&creator, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    status = fc_spsc_shm_open(&opener, name, size * 2);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_SIZE_MISMATCH);

    fc_spsc_shm_close(&creator);
    fc_spsc_shm_unlink(name);
}
#endif

TEST(test_spsc_shm_multiple_readers) {
    fc_spsc_shm_t writer, reader1, reader2;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 4096;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&writer, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    uint32_t* writer_base = (uint32_t*)fc_spsc_shm_base(&writer);
    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        writer_base[i] = (uint32_t)i;
    }

    status = fc_spsc_shm_open(&reader1, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    status = fc_spsc_shm_open(&reader2, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    uint32_t* reader1_base = (uint32_t*)fc_spsc_shm_base(&reader1);
    uint32_t* reader2_base = (uint32_t*)fc_spsc_shm_base(&reader2);

    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        ASSERT_EQ(reader1_base[i], (uint32_t)i);
        ASSERT_EQ(reader2_base[i], (uint32_t)i);
    }

    fc_spsc_shm_close(&reader2);
    fc_spsc_shm_close(&reader1);
    fc_spsc_shm_close(&writer);
    fc_spsc_shm_unlink(name);
}

TEST(test_spsc_shm_close_invalid) {
    fc_spsc_shm_t shm;
    memset(&shm, 0, sizeof(shm));

    fc_spsc_shm_status_t status = fc_spsc_shm_close(&shm);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_INVALID_ARG);
}

#ifndef _WIN32
TEST(test_spsc_shm_unlink) {
    fc_spsc_shm_t shm;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 4096;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&shm, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    fc_spsc_shm_close(&shm);

    status = fc_spsc_shm_unlink(name);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    status = fc_spsc_shm_open(&shm, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_ERR_OPEN);
}
#endif

TEST(test_spsc_shm_pattern_write_read) {
    fc_spsc_shm_t writer, reader;
    char name[256];
    generate_unique_name(name, sizeof(name));
    const size_t size = 8192;

    fc_spsc_shm_status_t status = fc_spsc_shm_create(&writer, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    uint8_t* writer_base = (uint8_t*)fc_spsc_shm_base(&writer);
    const uint8_t pattern[] = {0xDE, 0xAD, 0xBE, 0xEF};
    for (size_t i = 0; i < size; i++) {
        writer_base[i] = pattern[i % sizeof(pattern)];
    }

    status = fc_spsc_shm_open(&reader, name, size);
    ASSERT_EQ(status, FC_SPSC_SHM_OK);

    uint8_t* reader_base = (uint8_t*)fc_spsc_shm_base(&reader);
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(reader_base[i], pattern[i % sizeof(pattern)]);
    }

    fc_spsc_shm_close(&reader);
    fc_spsc_shm_close(&writer);
    fc_spsc_shm_unlink(name);
}

void register_spsc_shm_tests(void) {
    srand((unsigned int)time(NULL));

    RUN_TEST(test_spsc_shm_create_basic);
    RUN_TEST(test_spsc_shm_create_invalid_args);
    RUN_TEST(test_spsc_shm_create_and_open);
    RUN_TEST(test_spsc_shm_data_exchange);
    RUN_TEST(test_spsc_shm_large_segment);
    RUN_TEST(test_spsc_shm_open_nonexistent);
#ifndef _WIN32
    RUN_TEST(test_spsc_shm_size_mismatch);
#endif
    RUN_TEST(test_spsc_shm_multiple_readers);
    RUN_TEST(test_spsc_shm_close_invalid);
#ifndef _WIN32
    RUN_TEST(test_spsc_shm_unlink);
#endif
    RUN_TEST(test_spsc_shm_pattern_write_read);
}
