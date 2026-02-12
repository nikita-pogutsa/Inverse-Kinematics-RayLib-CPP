cmake_minimum_required(VERSION 3.16)
project(RaylibApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RaylibApp main.cpp)

# Add raylib from source checkout; adjust path if your raylib is elsewhere
add_subdirectory(C:/raylib/raylib ${CMAKE_BINARY_DIR}/raylib-build)

target_link_libraries(RaylibApp PRIVATE raylib)

# On Windows, raylib pulls required system libs automatically via its CMake, no extra setup needed.
