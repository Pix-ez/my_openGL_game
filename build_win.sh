
#!/usr/bin/env bash
set -e

BUILD_DIR=build-win


PROJECT_ROOT=$(pwd)
TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/mingw-w64.cmake"
ASSIMP_PREFIX="$PROJECT_ROOT/vendor/assimp-5.4.1-win64"

# We tell CMake where to find our custom-compiled SDL3 package.
# CMAKE_PREFIX_PATH is the standard way to do this.
SDL3_PREFIX="$PROJECT_ROOT/vendor/SDL3/x86_64-w64-mingw32"
SDL3_CONFIG_DIR="$PROJECT_ROOT/vendor/SDL3/x86_64-w64-mingw32/lib/cmake/SDL3"

if [ "$1" = "clean" ]; then
  echo "🧹 Cleaning build directory '$BUILD_DIR'..."
  rm -rf "$BUILD_DIR"
fi

if [ ! -d "$SDL3_PREFIX" ]; then
    echo "Error: SDL3 directory not found at: $SDL3_PREFIX"
    exit 1
fi

# Clean and create build directory
# rm -rf build-win
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
# We use CMAKE_PREFIX_PATH to point to our vendor libraries.
cmake .. \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSDL3_DIR="$SDL3_CONFIG_DIR" \
  -DCMAKE_PREFIX_PATH="$SDL3_PREFIX;$ASSIMP_PREFIX"
# Build the project
cmake --build . --parallel

echo ""
echo "Build complete! Your executable and SDL3.dll are in the build-win/ directory."