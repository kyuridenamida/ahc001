cmake_minimum_required(VERSION 3.17)
project(ahc001)

add_definitions(-DCLION)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -s -O3 -fsanitize=address -fsanitize=undefined")

add_executable(ahc001 src/main.cpp)
include_directories(src)