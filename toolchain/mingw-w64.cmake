# toolchain/mingw-w64.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross-compilers
set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# --- MODIFIED SECTION ---
# Base sysroot for the MinGW toolchain
set(MINGW_SYSROOT /usr/x86_64-w64-mingw32)

# Set the CMAKE_FIND_ROOT_PATH. This is a list.
# It includes the main sysroot and any custom paths passed from the command line.
# CMake uses semicolons to separate paths in a list.
set(CMAKE_FIND_ROOT_PATH ${MINGW_SYSROOT})
if(CUSTOM_FIND_ROOT_PATH)
  # Append the custom path to the list
  list(APPEND CMAKE_FIND_ROOT_PATH ${CUSTOM_FIND_ROOT_PATH})
endif()
# --- END MODIFIED SECTION ---

# Adjust the default behavior of the FIND_XXX() commands
# This forces CMake to search only within the CMAKE_FIND_ROOT_PATH directories.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)