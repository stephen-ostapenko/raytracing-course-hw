cmake_minimum_required(VERSION 3.11)
project(raytracing CXX)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

include_directories(${PROJECT_ROOT} ${PROJECT_ROOT}/include)

add_executable(
	main
	include/primitives.h src/primitives.cpp
	include/scene.h src/scene.cpp
	include/image.h src/image.cpp
	src/main.cpp
)
