cmake_minimum_required(VERSION 3.20)

project(
    StackPro
    VERSION 1.0.0
    LANGUAGES CXX
)


# ============================================================================
# C++ Configuration
# ============================================================================

# Configure the C++ language standard.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================================
# FunctionPro (Header-only Library)
# ============================================================================

# Define the header-only StackPro library.
add_library(StackPro INTERFACE)

# Expose the public include directory to consumers.
target_include_directories(
    StackPro
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Require C++23 for all targets that consume StackPro.
target_compile_features(
    StackPro
    INTERFACE
    cxx_std_23
)

# ============================================================================
# Tests
# ============================================================================

# Build the test executable.
add_executable(
    tests
    tests/test_all.cpp
    tests/test_constructor.cpp
    tests/test_move.cpp
    tests/test_memory.cpp
    tests/test_lifecycle.cpp
    tests/test_state.cpp
    tests/test_introspection.cpp
    tests/test_stats.cpp
    tests/test_scope.cpp
)

# Link StackPro to the test executable.
target_link_libraries(
    tests
    PRIVATE
    StackPro
)

# ============================================================================
# Benchmarks
# ============================================================================

# Build the benchmark executable.
add_executable(
    benchmarks
    benchmarks/bench_all.cpp
    benchmarks/bench_allocation.cpp
    benchmarks/bench_lifecycle.cpp
    benchmarks/bench_marker.cpp
    benchmarks/bench_scope.cpp
    benchmarks/bench_reset.cpp
)

# Link StackPro to the benchmark executable.
target_link_libraries(
    benchmarks
    PRIVATE
    StackPro
)


# ============================================================================
# Examples
# ============================================================================

# Build the basic usage example.
add_executable(
    example_basic
    examples/example_basic.cpp
)

# Link StackPro to the basic usage example.
target_link_libraries(
    example_basic
    PRIVATE
    StackPro
)

# Build the object lifecycle example.
add_executable(
    example_lifecycle
    examples/example_lifecycle.cpp
)

# Link StackPro to the object lifecycle example.
target_link_libraries(
    example_lifecycle
    PRIVATE
    StackPro
)

# Build the StackScope example.
add_executable(
    example_scope
    examples/example_scope.cpp
)

# Link StackPro to the StackScope example.
target_link_libraries(
    example_scope
    PRIVATE
    StackPro
)

# Build the statistics example.
add_executable(
    example_stats
    examples/example_stats.cpp
)

# Link StackPro to the statistics example.
target_link_libraries(
    example_stats
    PRIVATE
    StackPro
)


# ============================================================================
# Installation
# ============================================================================

# Install the StackPro target and export it for downstream projects.
install(
    TARGETS StackPro
    EXPORT StackProTargets
)

# Install the exported CMake targets.
install(
    EXPORT StackProTargets
    FILE StackProTargets.cmake
    NAMESPACE StackPro::
    DESTINATION lib/cmake/StackPro
)

# Install the public header files.
install(
    DIRECTORY include/
    DESTINATION include
)


# ============================================================================
# Packaging
# ============================================================================

include(CMakePackageConfigHelpers)

# Generate the package configuration file.
configure_package_config_file(
    cmake/StackProConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/StackProConfig.cmake
    INSTALL_DESTINATION lib/cmake/StackPro
)

# Generate the package version file.
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/StackProConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the package configuration files.
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/StackProConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/StackProConfigVersion.cmake
    DESTINATION lib/cmake/StackPro
)
