cmake_minimum_required(VERSION 3.14)
project(skymesh_satellite_os_core CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    target_compile_options(skymesh_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
    target_compile_options(skymesh_core PRIVATE /W4 /WX)
endif()

# Install library
install(TARGETS skymesh_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

# Download and configure Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Tests
add_executable(skymesh_core_tests
    tests/orbital_task_manager_test.cpp
)

# Link test executable with Google Test and our core library
target_link_libraries(skymesh_core_tests
    PRIVATE 
    gtest
    gtest_main
    skymesh_core
    Threads::Threads
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(skymesh_core_tests)

# Add a custom target for running tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS skymesh_core_tests
)

