cmake_minimum_required(VERSION 3.10)
project(WaveLogic VERSION 1.0.0)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find necessary packages (GLFW)
# On Windows/MinGW, ensure GLFW is in your PATH or CMAKE_PREFIX_PATH
include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8 # Or the latest stable tag
)

# This prevents GLFW from building its own documentation and tests
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)



# Add the core library folder
add_subdirectory(core)

# Add opengl_renderer example
add_subdirectory(examples/opengl_renderer)