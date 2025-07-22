
# cmake_minimum_required(VERSION 3.15)
# project(MyOpenglAPP LANGUAGES C CXX)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # For Windows cross-compilation with MinGW
# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
# endif()

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# # ===================================================================
# # DEPENDENCY DEFINITIONS
# # ===================================================================

# # --- Assimp (Find our pre-compiled package) ---
# # Set a hint path for find_package based on the operating system.
# if(WIN32)
#     # Update this path if your Windows install has a versioned folder too
#     set(Assimp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/local_install/windows/lib/cmake/assimp-6.0)
# else()
#     # This is the EXACT path you found!
#     set(Assimp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/local_install/linux/lib/cmake/assimp-6.0)
# endif()

# find_package(Assimp REQUIRED)

# # find_package(Assimp REQUIRED HINTS ${ASSIMP_HINT_DIR})

# # --- Header-Only Libraries (GLM, STB_Image) ---
# # Create a special "INTERFACE" library. It's just a named group of properties.
# # add_library(header_only_libs INTERFACE)roject) ---
# # This is the modern, robust, and correct way for this situation.
# # It guarantees compiler compatibility.
#     # --- Assimp Configuration (Safer & Still Fast) ---
# # set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force Assimp to build as a static library")
# # set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable building Assimp's tests")
# # set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable building the command-line tools")
# # set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable all model format exporters")

# # CRITICAL FIX: Force Assimp to build its own zlib. This prevents conflicts
# # when cross-compiling where it might find your Linux zlib.
# # set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib from source")

# # This command tells CMake to build the source code in vendor/assimp
# # add_subdirectory(vendor/assimp)

# add_library(header_only_libs INTERFACE)
# # This is the most direct way to specify an include path.
# target_include_directories(header_only_libs INTERFACE
#     ${CMAKE_CURRENT_SOURCE_DIR}/vendor
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )
# add_library(stb_image_lib STATIC src/stb_image_impl.cpp)

# # --- ImGui (Static library) ---
# add_library(imgui STATIC
#     vendor/imgui/imgui.cpp
#     vendor/imgui/imgui_draw.cpp
#     vendor/imgui/imgui_tables.cpp
#     vendor/imgui/imgui_widgets.cpp
#     vendor/imgui/backends/imgui_impl_sdl3.cpp
#     vendor/imgui/backends/imgui_impl_opengl3.cpp
# )
# target_include_directories(imgui PUBLIC
#     vendor/imgui
#     vendor/imgui/backends
# )
# target_link_libraries(imgui PUBLIC SDL3::SDL3)


# # --- GLAD (Static library) ---
# add_library(glad STATIC src/glad.c)
# target_include_directories(glad PUBLIC include)


# # ===================================================================
# # MAIN EXECUTABLE
# # ===================================================================

# find_package(SDL3 REQUIRED CONFIG)

# add_executable(${PROJECT_NAME}
#     src/main.cpp
#     src/shader.cpp
  
#     # Add camera.cpp, inputController.cpp etc. here
# )

# # Add include paths for our own source code
# target_include_directories(${PROJECT_NAME} PRIVATE src ${ASSIMP_INCLUDE_DIRS}  )

# # ===================================================================
# # LINKING
# # ===================================================================
# # add_dependencies(${PROJECT_NAME} assimp)

# # Link all our dependencies to the main executable
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     SDL3::SDL3
#     imgui
#     glad
#     stb_image_lib
#     header_only_libs  # <-- This passes the include paths for GLM and STB
#     ${ASSIMP_LIBRARIES}
# )

# # Link against the platform-specific OpenGL library
# if(WIN32)
#     target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 )

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

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# ===================================================================
# DEPENDENCY DEFINITIONS
# ===================================================================
# --- Platform-Aware ZLIB Handling ---
if(WIN32)
    # For Windows build, compile ZLIB from source to guarantee compatibility.
    # This creates a target named 'zlibstatic'.
    add_subdirectory(vendor/zlib)
    set(ZLIB_TARGET_NAME zlibstatic) # Store the target name in a variable
else()
    # For Linux, find the system library, which we know works.
    # This creates the imported target 'ZLIB::ZLIB'.
    find_package(ZLIB REQUIRED)
    set(ZLIB_TARGET_NAME ZLIB::ZLIB) # Store the target name in a variable
endif()

# find_package(ZLIB REQUIRED)
add_library(assimp STATIC IMPORTED GLOBAL)

target_include_directories(assimp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# --- Assimp (Find our pre-compiled package) ---
# Set a hint path for find_package based on the operating system.
# if(WIN32)
#     # Update this path if your Windows install has a versioned folder too
#     set(Assimp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/local_install/windows/lib/cmake/assimp-6.0)
# else()
#     # This is the EXACT path you found!
#     set(Assimp_DIR ${CMAKE_CURRENT_SOURCE_DIR}/local_install/linux/lib/cmake/assimp-6.0)
# endif()


# Now, set the platform-specific library file location.
if(WIN32)
    set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/libassimp.a
    )
else()
    set_target_properties(assimp PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libassimp.a
    )
endif()
# target_link_libraries(assimp INTERFACE ZLIB::ZLIB)
# find_package(Assimp REQUIRED)

# find_package(Assimp REQUIRED HINTS ${ASSIMP_HINT_DIR})

# --- Header-Only Libraries (GLM, STB_Image) ---
# Create a special "INTERFACE" library. It's just a named group of properties.
# add_library(header_only_libs INTERFACE)roject) ---
# This is the modern, robust, and correct way for this situation.
# It guarantees compiler compatibility.
    # --- Assimp Configuration (Safer & Still Fast) ---
# set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force Assimp to build as a static library")
# set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable building Assimp's tests")
# set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Disable building the command-line tools")
# set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable all model format exporters")

# CRITICAL FIX: Force Assimp to build its own zlib. This prevents conflicts
# when cross-compiling where it might find your Linux zlib.
# set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "Build zlib from source")

# This command tells CMake to build the source code in vendor/assimp
# add_subdirectory(vendor/assimp)

add_library(header_only_libs INTERFACE)
# This is the most direct way to specify an include path.
target_include_directories(header_only_libs INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_library(stb_image_lib STATIC src/stb_image_impl.cpp)

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
    src/utils.cpp
  
    # Add camera.cpp, inputController.cpp etc. here
)

# Add include paths for our own source code
target_include_directories(${PROJECT_NAME} PRIVATE src )

# ===================================================================
# LINKING
# ===================================================================
# add_dependencies(${PROJECT_NAME} assimp)

# Link all our dependencies to the main executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    imgui
    glad
    stb_image_lib
    header_only_libs  # <-- This passes the include paths for GLM and STB
    # ${ASSIMP_LIBRARIES}
    assimp
    ZLIB::ZLIB
)

# Link against the platform-specific OpenGL library
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 )

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