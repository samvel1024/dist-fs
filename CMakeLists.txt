cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)
project(distfs)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)