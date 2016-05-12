cmake_minimum_required (VERSION 3.5)

project (ZenBenchTests)

set (CMAKE_CXX_STANDARD 11)

include_directories (../include)
add_executable (example main.cpp other.cpp)
