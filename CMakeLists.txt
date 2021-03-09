cmake_minimum_required(VERSION 3.17)
project(ahc001)

add_definitions(-DCLION)
set(CMAKE_CXX_STANDARD 14 -O3)

add_executable(ahc001 src/main.cpp)
