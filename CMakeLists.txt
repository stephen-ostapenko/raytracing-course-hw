cmake_minimum_required(VERSION 3.11)
project(raytracing)
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${PROJECT_ROOT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -pedantic")

add_executable(main main.cpp)
