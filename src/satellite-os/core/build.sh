#!/bin/bash
# Build script for SkyMesh Satellite-OS Core Module

set -e

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir -p build
fi

# Navigate to build directory
cd build

# Configure with CMake
echo "Configuring SkyMesh Satellite-OS Core..."
cmake ..

# Build the library and tests
echo "Building SkyMesh Satellite-OS Core..."
cmake --build .

# Run tests if they were built
if [ -f "skymesh_core_tests" ]; then
    echo "Running tests..."
    ./skymesh_core_tests
fi

echo "Build completed successfully!"

