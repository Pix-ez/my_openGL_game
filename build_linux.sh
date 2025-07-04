#!/usr/bin/env bash
set -e  # Exit on error

# Clean and create build directory
rm -rf build-linux
mkdir -p build-linux
cd build-linux


# Configure CMake for native Linux build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DSDL3_DIR=/usr/lib/cmake/SDL3

# Build using all CPU cores
cmake --build . --parallel
