# ===================================================================
# PROJECT SETUP
# ===================================================================
cmake_minimum_required(VERSION 3.16)
project(MyOpenglAPP LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # For .exe
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # For static libs (.a)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # For shared libs (.so/.dll)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(PROJECT_ROOT ${PROJECT_SOURCE_DIR})


# ===================================================================
# DEPENDENCY MANAGEMENT WITH FETCHCONTENT
# ===================================================================
include(FetchContent)
set(FETCHCONTENT_QUIET ON) # Set to ON to hide download/build output


# This creates external/ directory that survives clean builds
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external" CACHE PATH "FetchContent base directory")

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON) # Don't check for updates every time

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.1
    GIT_SHALLOW    ON
)

# Your existing configuration
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_USE_OWN_ZLIB ON CACHE BOOL "" FORCE)

if(WIN32)
    set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(assimp)
# # This is the robust way to handle cross-compilation.
# FetchContent_GetProperties(assimp)


if(WIN32)
    message(STATUS "--- Cross-compilation DEBUG ---")
    message(STATUS "The value of assimp_SOURCE_DIR is: '${assimp_SOURCE_DIR}'")
    message(STATUS "The value of assimp_BINARY_DIR is: '${assimp_BINARY_DIR}'")
    message(STATUS "-------------------------------")
endif()


# --- SDL3 ---
# This is the modern way to handle SDL3. It will find the SDL3Config.cmake file
# that should be in your vendor/SDL3 directory and set up the SDL3::SDL3 target.
find_package(SDL3 REQUIRED CONFIG)


# ===================================================================
# OUR LIBRARIES (Imgui, GLAD, STB)
# ===================================================================

# --- GLAD ---
# GLAD source file is in 'src' and its headers are in 'include'
add_library(glad STATIC ${PROJECT_ROOT}/src/core/glad.c)
target_include_directories(glad PUBLIC ${PROJECT_ROOT}/include)



# --- ImGui ---
# This setup for ImGui is good. We create a static library from its sources.
# --- ImGui ---
add_library(imgui STATIC
    ${PROJECT_ROOT}/vendor/imgui/imgui.cpp
    ${PROJECT_ROOT}/vendor/imgui/imgui_draw.cpp
    ${PROJECT_ROOT}/vendor/imgui/imgui_tables.cpp
    ${PROJECT_ROOT}/vendor/imgui/imgui_widgets.cpp
    ${PROJECT_ROOT}/vendor/imgui/backends/imgui_impl_sdl3.cpp
    ${PROJECT_ROOT}/vendor/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${PROJECT_ROOT}/vendor/imgui
    ${PROJECT_ROOT}/vendor/imgui/backends
)

# We need to link ImGui against SDL3 as it uses it in the backend.
target_link_libraries(imgui PUBLIC SDL3::SDL3)


# --- Our Core Game Library ---
file(GLOB_RECURSE CORE_SOURCES
    CONFIGURE_DEPENDS
    "${PROJECT_ROOT}/src/core/*.cpp"
)

add_library(Core STATIC
    ${CORE_SOURCES}
    "${PROJECT_ROOT}/src/core/tinyfiledialogs.c"
)

# We add include/core so it can find "stb_image.h" directly.
target_include_directories(Core PUBLIC
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/core
    ${PROJECT_ROOT}/vendor
     ${assimp_SOURCE_DIR}/include
    ${assimp_BINARY_DIR}/include # <-- This path is for generated headers like config.h
)



target_link_libraries(Core PUBLIC assimp imgui)

# ===================================================================
# MAIN EXECUTABLE
# ===================================================================

add_executable(${PROJECT_NAME}
    ${PROJECT_ROOT}/src/main.cpp
)



# ===================================================================
# LINKING
# ===================================================================

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    Core
    imgui
    glad
   
)

# --- Platform-Specific Linking ---
if(WIN32)
    # For Windows, we link to the standard opengl32 library.
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

    # Static linking options - this fixes the DLL issue
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++ -lpthread"
    )
    #target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    # This ensures that your Windows executable can find the SDL3.dll at runtime.
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3-shared>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying SDL3.dll to output directory"
    )
else()
    # For Linux, we find and link to OpenGL.
    find_package(OpenGL REQUIRED)
    # --- ADD THESE TWO LINES FOR TINYFILEDIALOGS ---
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZENITY zenity)

    target_link_libraries(${PROJECT_NAME} PRIVATE 
        OpenGL::GL
        # --- AND ADD THIS LINE ---
        ${ZENITY_LIBRARIES}
    )
endif()
