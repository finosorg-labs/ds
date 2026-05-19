#
# benchmarks/CMakeLists.txt - Benchmark configuration for ds module
#

# Benchmark framework library
add_library(bench_framework OBJECT
    bench_framework.c
)

target_include_directories(bench_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/modules/platform/include
)

target_link_libraries(bench_framework
    PUBLIC
        finkit_ds_static
)

# Benchmark executable
add_executable(ds_benchmarks
    bench_main.c
    bench_ds.c
    bench_bloom_filter.c
    bench_roaring_bitmap.c
    bench_ring_buffer.c
)

# Link with benchmark framework and ds library
target_link_libraries(ds_benchmarks
    PRIVATE
        bench_framework
        finkit_ds_static
        m
)

target_compile_definitions(ds_benchmarks PRIVATE
    FC_STATIC_BUILD
)

# Include directories
target_include_directories(ds_benchmarks
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/modules/platform/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for benchmarks
target_compile_options(ds_benchmarks PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)

# Installation
install(TARGETS ds_benchmarks
    RUNTIME DESTINATION bin
)
