# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build

# Include any dependencies generated for this target.
include CMakeFiles/skymesh_core_tests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/skymesh_core_tests.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/skymesh_core_tests.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/skymesh_core_tests.dir/flags.make

CMakeFiles/skymesh_core_tests.dir/codegen:
.PHONY : CMakeFiles/skymesh_core_tests.dir/codegen

CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o: CMakeFiles/skymesh_core_tests.dir/flags.make
CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o: /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/orbital_task_manager_test.cpp
CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o: CMakeFiles/skymesh_core_tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o -MF CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o.d -o CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o -c /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/orbital_task_manager_test.cpp

CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/orbital_task_manager_test.cpp > CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.i

CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/orbital_task_manager_test.cpp -o CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.s

CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o: CMakeFiles/skymesh_core_tests.dir/flags.make
CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o: /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/test_radiation_hardening.cpp
CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o: CMakeFiles/skymesh_core_tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o -MF CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o.d -o CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o -c /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/test_radiation_hardening.cpp

CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/test_radiation_hardening.cpp > CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.i

CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/tests/test_radiation_hardening.cpp -o CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.s

# Object files for target skymesh_core_tests
skymesh_core_tests_OBJECTS = \
"CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o" \
"CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o"

# External object files for target skymesh_core_tests
skymesh_core_tests_EXTERNAL_OBJECTS =

skymesh_core_tests: CMakeFiles/skymesh_core_tests.dir/tests/orbital_task_manager_test.cpp.o
skymesh_core_tests: CMakeFiles/skymesh_core_tests.dir/tests/test_radiation_hardening.cpp.o
skymesh_core_tests: CMakeFiles/skymesh_core_tests.dir/build.make
skymesh_core_tests: libskymesh_core.a
skymesh_core_tests: lib/libgtest.a
skymesh_core_tests: lib/libgtest_main.a
skymesh_core_tests: lib/libgmock.a
skymesh_core_tests: lib/libgmock_main.a
skymesh_core_tests: lib/libgmock.a
skymesh_core_tests: lib/libgtest.a
skymesh_core_tests: CMakeFiles/skymesh_core_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable skymesh_core_tests"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/skymesh_core_tests.dir/link.txt --verbose=$(VERBOSE)
	/usr/local/bin/cmake -D TEST_TARGET=skymesh_core_tests -D TEST_EXECUTABLE=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/skymesh_core_tests -D TEST_EXECUTOR= -D TEST_WORKING_DIR=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build -D TEST_EXTRA_ARGS= -D TEST_PROPERTIES= -D TEST_PREFIX= -D TEST_SUFFIX= -D TEST_FILTER= -D NO_PRETTY_TYPES=FALSE -D NO_PRETTY_VALUES=FALSE -D TEST_LIST=skymesh_core_tests_TESTS -D CTEST_FILE=/Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/skymesh_core_tests[1]_tests.cmake -D TEST_DISCOVERY_TIMEOUT=5 -D TEST_DISCOVERY_EXTRA_ARGS= -D TEST_XML_OUTPUT_DIR= -P /usr/local/share/cmake/Modules/GoogleTestAddTests.cmake

# Rule to build all files generated by this target.
CMakeFiles/skymesh_core_tests.dir/build: skymesh_core_tests
.PHONY : CMakeFiles/skymesh_core_tests.dir/build

CMakeFiles/skymesh_core_tests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/skymesh_core_tests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/skymesh_core_tests.dir/clean

CMakeFiles/skymesh_core_tests.dir/depend:
	cd /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build /Users/alex/devving/code-lobo/PROJECTS/SkyMesh/src/satellite-os/core/build/CMakeFiles/skymesh_core_tests.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/skymesh_core_tests.dir/depend

