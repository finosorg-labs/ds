#
# tests/CMakeLists.txt - Test configuration for ds module
#

# Test executable
add_executable(ds_tests
    test_framework.c
    test_main.c
    test_ds.c
    test_bloom_filter.c
    test_roaring_bitmap.c
    test_ring_buffer.c
    test_ring_buffer_mt.c
    test_generic_ring_buffer.c
    test_mem_pool.c
    test_arena.c
    test_spsc.c
    test_spsc_shm.c
)

# Link with ds library
target_link_libraries(ds_tests
    PRIVATE
        finkit_ds_static_base
        finkit::platform
        m
        atomic
        pthread
)

target_compile_definitions(ds_tests PRIVATE
    FC_STATIC_BUILD
)

# Include directories
target_include_directories(ds_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/modules/platform/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for tests
target_compile_options(ds_tests PRIVATE -Wall -Wextra -Wpedantic)

# Run tests with ctest
add_test(
    NAME "ds_tests"
    COMMAND ds_tests
)

# Installation
install(TARGETS ds_tests
    RUNTIME DESTINATION bin
)
