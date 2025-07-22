
#!/usr/bin/env bash
set -e  # Exit on error

BUILD_DIR=build-linux

if [ "$1" = "clean" ]; then
  echo "🧹 Cleaning build directory '$BUILD_DIR'..."
  rm -rf "$BUILD_DIR"
fi

# Create build dir if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# (Re)configure only when needed
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DSDL3_DIR=/usr/lib/cmake/SDL3

# Build using all CPU cores
cmake --build . --parallel 
