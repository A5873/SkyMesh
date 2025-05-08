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

# Download and configure Google Test using direct URL
include(FetchContent)

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
    )
    
    # Ensure STL features are available
    add_definitions(
        -D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
endif()

# Force CMake policy for Google Test
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

# Make sure Google Test aligns with our CMake policies
if(TARGET gtest)
  message(STATUS "GoogleTest found and configured")
  set_property(TARGET gtest PROPERTY CXX_STANDARD 17)
  set_property(TARGET gtest_main PROPERTY CXX_STANDARD 17)
  set_property(TARGET gmock PROPERTY CXX_STANDARD 17)
  set_property(TARGET gmock_main PROPERTY CXX_STANDARD 17)
endif()

# Configure Google Test
include(GoogleTest)

# Tests
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
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

# Ensure the test executable has the correct STL feature flags
target_compile_definitions(skymesh_core_tests
    PRIVATE
    GTEST_HAS_STD_TUPLE=1
    GTEST_USE_OWN_TR1_TUPLE=0
)

# Link test executable with Google Test and our core library
target_link_libraries(skymesh_core_tests
    PRIVATE 
    skymesh_core
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    Threads::Threads
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

