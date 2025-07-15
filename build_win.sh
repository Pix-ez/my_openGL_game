# #!/usr/bin/env bash
# set -e

# PROJECT_ROOT=$(pwd)
# TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/mingw-w64.cmake"

# # The path to your SDL3 library installation prefix
# # This is the directory that contains the bin/, include/, and lib/ folders.
# SDL3_MINGW_PREFIX="$PROJECT_ROOT/vendor/SDL3/x86_64-w64-mingw32"

# # --- Add a check to make sure the directory exists! ---
# if [ ! -d "$SDL3_MINGW_PREFIX" ]; then
#     echo "Error: SDL3 directory not found at: $SDL3_MINGW_PREFIX"
#     echo "Please check the folder name inside your 'vendor' directory and update the script."
#     exit 1
# fi

# # Clean and create build directory
# rm -rf build-win
# mkdir -p build-win
# cd build-win

# # Configure CMake
# # Pass the vendor path using the new variable we defined in the toolchain file.
# cmake .. \
#   -G Ninja \
#   -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DCUSTOM_FIND_ROOT_PATH="$SDL3_MINGW_PREFIX"

# # Build the project
# cmake --build . --parallel

# echo ""
# echo "Build complete! Your executable and SDL3.dll are in the build-win/ directory."


#!/usr/bin/env bash
set -e

PROJECT_ROOT=$(pwd)
TOOLCHAIN_FILE="$PROJECT_ROOT/toolchain/mingw-w64.cmake"

# We tell CMake where to find our custom-compiled SDL3 package.
# CMAKE_PREFIX_PATH is the standard way to do this.
SDL3_MINGW_PREFIX="$PROJECT_ROOT/vendor/SDL3/x86_64-w64-mingw32"

if [ ! -d "$SDL3_MINGW_PREFIX" ]; then
    echo "Error: SDL3 directory not found at: $SDL3_MINGW_PREFIX"
    exit 1
fi

# Clean and create build directory
rm -rf build-win
mkdir -p build-win
cd build-win

# Configure CMake
# We use CMAKE_PREFIX_PATH to point to our vendor libraries.
cmake .. \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$SDL3_MINGW_PREFIX"

# Build the project
cmake --build . --parallel

echo ""
echo "Build complete! Your executable and SDL3.dll are in the build-win/ directory."