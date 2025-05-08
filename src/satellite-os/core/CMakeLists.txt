# CMakeLists.txt for the SkyMesh Satellite-OS Core Module
# Set minimum CMake version and policy
cmake_minimum_required(VERSION 3.14...3.22)
# Force new CMake policy behavior
cmake_policy(SET CMP0048 NEW)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Set policy version for subprojects
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# C++ Standard settings - must be at the top
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Additional compiler configuration
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -pedantic)
    # Add C++17 feature detection flags
    add_compile_options(-D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES)
endif()

# Project information
project(skymesh_satellite_os_core 
    VERSION 0.1.0
    DESCRIPTION "Core components of the SkyMesh satellite operating system"
    LANGUAGES CXX)

# Enable testing
enable_testing()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find dependencies
find_package(Threads REQUIRED)

# Library sources
set(SOURCES
    src/orbital_task_manager.cpp
    src/health_monitor.cpp
    src/power_manager.cpp
)

# Library headers
set(HEADERS
    include/skymesh/core/orbital_task_manager.h
    include/skymesh/core/health_monitor.h
    include/skymesh/core/power_manager.h
    include/skymesh/core/command_control.h
)

# Core library
add_library(skymesh_core ${SOURCES} ${HEADERS})
target_link_libraries(skymesh_core PUBLIC Threads::Threads)

# Set compiler warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(skymesh_core PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(skymesh_core PRIVATE /W4)
endif()

# Installation
install(TARGETS skymesh_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

# Create test_utils.h header with compatibility fixes
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_utils.h [[
#pragma once

// Compatibility shims for Google Test with C++17
namespace testing {
namespace internal {
    template <size_t... I>
    using IndexSequence = std::index_sequence<I...>;

    template <size_t N>
    using MakeIndexSequence = std::make_index_sequence<N>;
}  // namespace internal
}  // namespace testing
]])

# Download and configure Google Test using direct URL
include(FetchContent)

# Disable find_package for GTest to prevent system GTest interference
set(CMAKE_DISABLE_FIND_PACKAGE_GTest TRUE CACHE BOOL "Disable finding system-installed GTest" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_GMock TRUE CACHE BOOL "Disable finding system-installed GMock" FORCE)

# Force Google Test to use C++17
set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Require the C++ standard")
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Explicitly reset C++ standard settings before Google Test configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add more specific compiler definitions for Google Test
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_definitions(
        -DGTEST_USE_OWN_TR1_TUPLE=0
        -DGTEST_HAS_TR1_TUPLE=0
        -DGTEST_HAS_STD_TUPLE=1
        -DGTEST_LANG_CXX11=1
        -DGTEST_HAS_STREAM_REDIRECTION=1
        -DGTEST_HAS_CXXABI_H_=1
    )
    
    # Ensure STL features are available
    add_definitions(
        -D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        -D_GLIBCXX_USE_CXX11_ABI=1
    )
endif()

# Force CMake policy for Google Test
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)

# Use a specific GoogleTest commit known to work with C++17 and our environment
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # v1.10.0 with fixes
)

# Before making Google Test available, set properties that will affect its build
set(BUILD_GMOCK ON CACHE BOOL "Build GMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable Google Test installation" FORCE)

FetchContent_MakeAvailable(googletest)

# Configure Google Test
include(GoogleTest)
enable_testing()

# Output Google Test status
if(TARGET gtest)
  message(STATUS "GoogleTest found and configured")
endif()

add_executable(skymesh_core_tests
    tests/orbital_task_manager_test.cpp
    tests/test_radiation_hardening.cpp
)

# Explicitly require C++17 features for the test executable
target_compile_features(skymesh_core_tests PRIVATE cxx_std_17)

# Add explicit include directories for test executable
target_include_directories(skymesh_core_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}  # For our test_utils.h
    # Use the fetched Google Test directories with higher priority
    ${googletest_SOURCE_DIR}/googletest/include
    ${googletest_SOURCE_DIR}/googlemock/include
    # Fallback to standard locations
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

# Ensure the test executable has the correct STL feature flags
target_compile_definitions(skymesh_core_tests
    PRIVATE
    GTEST_HAS_STD_TUPLE=1
    GTEST_USE_OWN_TR1_TUPLE=0
    GTEST_LANG_CXX11=1
    GTEST_HAS_TR1_TUPLE=0
    # Disable problematic GMock features that may cause issues with C++17
    GMOCK_MOCK_MISMATCH_AT_COMPILE_TIME=0 
)

# If using Clang, add extra flags to ensure proper C++17 support
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(skymesh_core_tests PRIVATE
        -Wno-deprecated-declarations
        -Wno-deprecated-register
        -Wno-unused-macros
        -Wno-missing-field-initializers
    )
endif()

# Include our compatibility header in the test files
target_compile_options(skymesh_core_tests PRIVATE
    "-include${CMAKE_CURRENT_BINARY_DIR}/test_utils.h"
)

# Link test executable with Google Test and our core library
target_link_libraries(skymesh_core_tests
    PRIVATE 
    skymesh_core
    gtest
    gtest_main
    gmock
    gmock_main
    Threads::Threads
)
)

# Register tests with CTest
gtest_discover_tests(skymesh_core_tests)

# Add a custom target for running tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS skymesh_core_tests
)

# Output configuration summary
message(STATUS "SkyMesh Satellite-OS Core Configuration:")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Compiler:   ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Testing:        ON")

