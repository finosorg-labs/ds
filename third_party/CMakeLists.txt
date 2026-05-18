# =============================================================================
# third_party/CMakeLists.txt - Third-party library integration
# =============================================================================
# Integrates xxHash via their C source files.
# All third-party libraries are BSD-licensed.
# =============================================================================

# ---------------------------------------------------------------------------
# xxHash (BSD-2-Clause) - https://cyan4973.github.io/xxHash/
# ---------------------------------------------------------------------------
add_library(finkit_3rdparty_xxhash STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/xxhash/xxhash.c
)

target_include_directories(finkit_3rdparty_xxhash
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/xxhash
)

target_compile_options(finkit_3rdparty_xxhash PRIVATE -w)

# ---------------------------------------------------------------------------
# Combined third-party library for easy linking
# ---------------------------------------------------------------------------
add_library(finkit_3rdparty INTERFACE)
add_library(finkit::3rdparty ALIAS finkit_3rdparty)

target_link_libraries(finkit_3rdparty INTERFACE
    finkit_3rdparty_xxhash
)
