#!/bin/bash
# Main build script for SkyMesh project

set -e

echo "Building SkyMesh Satellite-OS Core module..."

# Make the build.sh script executable
chmod +x src/satellite-os/core/build.sh

# Navigate to core module and build
cd src/satellite-os/core
./build.sh

echo "Build process completed successfully!"

