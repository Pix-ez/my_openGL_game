
# cmake_minimum_required(VERSION 3.15)
# project(MyOpenglAPP LANGUAGES C CXX)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_subdirectory(vendor/glm)
# # BUNDLE THE C++ RUNTIME LIBRARIES INTO THE EXE (for Windows cross-compilation)
# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
# endif()

# find_package(SDL3 REQUIRED CONFIG)

# # ImGui
# add_library(imgui STATIC
#   vendor/imgui/imgui.cpp
#   vendor/imgui/imgui_draw.cpp
#   vendor/imgui/imgui_tables.cpp
#   vendor/imgui/imgui_widgets.cpp
#   vendor/imgui/backends/imgui_impl_sdl3.cpp
#   vendor/imgui/backends/imgui_impl_opengl3.cpp
# )
# target_include_directories(imgui PUBLIC
#   vendor/imgui
#   vendor/imgui/backends
# )
# target_link_libraries(imgui PUBLIC SDL3::SDL3)

# # --- GLAD INTEGRATION: START ---

# # 1. Add glad.c to the list of source files to be compiled for your executable.
# add_executable(${PROJECT_NAME}
#     src/main.cpp
#     src/glad.c
#     src/shader.cpp
#     # src/camera.h
#     # src/inputController.h
# )

# # 2. Tell your target where to find the GLAD header files (<glad/glad.h>).
# target_include_directories(${PROJECT_NAME} PRIVATE include src)

# # --- GLAD INTEGRATION: END ---

# # Link against SDL3 (as before)
# target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm imgui)

# # Link against the platform-specific OpenGL library (which GLAD needs)
# if(WIN32)
#     target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

#     add_custom_command(
#         TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         $<TARGET_FILE:SDL3::SDL3-shared>
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>
#         COMMENT "Copying SDL3.dll to output directory"
#     )
# else()
#     find_package(OpenGL REQUIRED)
#     target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
# endif()




cmake_minimum_required(VERSION 3.15)
project(MyOpenglAPP LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For Windows cross-compilation with MinGW
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# ===================================================================
# DEPENDENCY DEFINITIONS
# ===================================================================

# --- Header-Only Libraries (GLM, STB_Image) ---
# Create a special "INTERFACE" library. It's just a named group of properties.
add_library(header_only_libs INTERFACE)

# Add the include directories for GLM and STB to this group.
# This is the most direct way to specify an include path.
target_include_directories(header_only_libs INTERFACE
    # The compiler needs the path to the directory CONTAINING the "glm" folder
    # so that `#include <glm/glm.hpp>` works.
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- ImGui (Static library) ---
add_library(imgui STATIC
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/backends/imgui_impl_sdl3.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    vendor/imgui
    vendor/imgui/backends
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

# --- GLAD (Static library) ---
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)


# ===================================================================
# MAIN EXECUTABLE
# ===================================================================

find_package(SDL3 REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/shader.cpp
    # Add camera.cpp, inputController.cpp etc. here
)

# Add include paths for our own source code
target_include_directories(${PROJECT_NAME} PRIVATE src)

# ===================================================================
# LINKING
# ===================================================================

# Link all our dependencies to the main executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    imgui
    glad
    header_only_libs  # <-- This passes the include paths for GLM and STB
)

# Link against the platform-specific OpenGL library
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3-shared>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying SDL3.dll to output directory"
    )
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()