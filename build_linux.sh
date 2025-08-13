
# #!/usr/bin/env bash
# set -e  # Exit on error

# BUILD_DIR=build-linux

# if [ "$1" = "clean" ]; then
#   echo "🧹 Cleaning build directory '$BUILD_DIR'..."
#   rm -rf "$BUILD_DIR"
# fi

# # Create build dir if it doesn't exist
# mkdir -p "$BUILD_DIR"
# cd "$BUILD_DIR"

# # (Re)configure only when needed
# cmake .. \
#   -DCMAKE_BUILD_TYPE=Release \
#   -DSDL3_DIR=/usr/lib/cmake/SDL3

# # Build using all CPU cores
# cmake --build . --parallel 
#!/usr/bin/env bash
set -e  # Exit on error

# Default values
BUILD_TYPE="Debug"
BUILD_DIR=build-linux

# Normalize input
USER_BUILD_TYPE=$(echo "$1" | tr '[:upper:]' '[:lower:]')

# Handle 'clean' or 'debug/release' logic
if [ "$USER_BUILD_TYPE" = "clean" ]; then
  echo "🧹 Cleaning build directory '$BUILD_DIR'..."
  rm -rf "$BUILD_DIR"
  exit 0
elif [ "$USER_BUILD_TYPE" = "release" ]; then
  BUILD_TYPE="Release"
elif [ "$USER_BUILD_TYPE" != "" ] && [ "$USER_BUILD_TYPE" != "debug" ]; then
  echo "❌ Invalid build type: '$1'. Use 'debug', 'release', or 'clean'."
  exit 1
fi

echo "🔧 Build type: $BUILD_TYPE"

# Create build dir if it doesn't exist
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DSDL3_DIR=/usr/lib/cmake/SDL3

# Build using all CPU cores
cmake --build . --parallel 2 #-v
