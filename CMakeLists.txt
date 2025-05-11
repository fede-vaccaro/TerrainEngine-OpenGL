# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TerrainEngine LANGUAGES C CXX VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# download tiny_obj_loader.h
file(
    DOWNLOAD
    "https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/3bb554cf74428d7db13418b4aca1b9752a1d2be8/tiny_obj_loader.h"
    ${CMAKE_SOURCE_DIR}/Include/tiny_obj_loader.h
    EXPECTED_HASH SHA256=5088b18c4b0ccf42193a8fe89a6a22c71cc544cd0746b79e70c59dc5efed151f
    SHOW_PROGRESS
)

# download tiny_obj_loader.cc
file(
    DOWNLOAD
    "https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/3bb554cf74428d7db13418b4aca1b9752a1d2be8/tiny_obj_loader.cc"
    ${CMAKE_SOURCE_DIR}/Include/tiny_obj_loader.cc
    EXPECTED_HASH SHA256=b27322737fbd97b615412efd404d20d4a9b0a6556cada15ca4b77c7447b38bd2
    SHOW_PROGRESS
)


# download outcome.hpp
file(
    DOWNLOAD
    "https://raw.githubusercontent.com/ned14/outcome/8b25c1713ebb2590981d8c4bc176d028026bdf8a/single-header/outcome.hpp"
    ${CMAKE_SOURCE_DIR}/Include/outcome.hpp
    EXPECTED_HASH SHA256=2b9d92acca21cc9294e138c848630c8992edc3540f4fb4819a3b396e9d8a2ccc
    SHOW_PROGRESS
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/DrawableObjects
)

# Compile glad.c into a static library
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/Compile/glad.c)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# Compile stb_image.cpp into a static library
add_library(stb_image STATIC ${CMAKE_SOURCE_DIR}/Compile/stb_image.cpp)

# Compile stb_image_write.cpp into a static library
add_library(stb_image_write STATIC ${CMAKE_SOURCE_DIR}/Compile/stb_image_write.cpp)

set(IMGUI_SRC
    # core
    ${CMAKE_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp

    # backends
    ${CMAKE_SOURCE_DIR}/imgui/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/imgui/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SRC})
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Compile tiny_obj_loader into a static library
add_library(tiny_obj_loader STATIC ${CMAKE_SOURCE_DIR}/Include/tiny_obj_loader.cc)

# Add source files
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/Engine/*.cpp)
file(GLOB_RECURSE SRC_FILES_DRAWABLEOBJECTS ${CMAKE_SOURCE_DIR}/DrawableObjects/*.cpp)


# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${SRC_FILES_DRAWABLEOBJECTS} ${CMAKE_SOURCE_DIR}/Main.cpp )

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glad
    stb_image
    stb_image_write
    imgui
    tiny_obj_loader
)

# Add compile options and definitions
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -g -fno-omit-frame-pointer -O3)
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/resources/")

# Set include directories for the target
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Engine)

# Print status messages
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Source files: ${SRC_FILES}")
