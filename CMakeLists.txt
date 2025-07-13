
cmake_minimum_required(VERSION 3.15)
project(MyOpenglAPP LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(vendor/glm)
# BUNDLE THE C++ RUNTIME LIBRARIES INTO THE EXE (for Windows cross-compilation)
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

find_package(SDL3 REQUIRED CONFIG)

# --- GLAD INTEGRATION: START ---

# 1. Add glad.c to the list of source files to be compiled for your executable.
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/glad.c
    src/shader.cpp
)

# 2. Tell your target where to find the GLAD header files (<glad/glad.h>).
target_include_directories(${PROJECT_NAME} PRIVATE include)

# --- GLAD INTEGRATION: END ---

# Link against SDL3 (as before)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm)

# Link against the platform-specific OpenGL library (which GLAD needs)
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