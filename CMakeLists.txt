cmake_minimum_required(VERSION 3.17)

project(slip-angle VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include("cmake/GetOpenCV.cmake")
#include("cmake/FetchOpenCV.cmake")

add_subdirectory("src/")