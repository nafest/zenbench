cmake_minimum_required (VERSION 3.5)
include (ExternalProject)

# get and build gtest
ExternalProject_Add(
    googletest
    DOWNLOAD_COMMAND git clone https://github.com/google/googletest.git
    INSTALL_COMMAND ""
)
add_library (libgtest IMPORTED STATIC GLOBAL)
add_library (libgtest_main IMPORTED STATIC GLOBAL)
add_dependencies (libgtest googletest)
add_dependencies (libgtest_main googletest)

ExternalProject_Get_Property (googletest source_dir binary_dir)
set_target_properties (libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
   # "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/googletest/include"
)
set_target_properties (libgtest_main PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest_main.a"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
   # "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/googletest/include"
)

set (GTEST_INCLUDES "${source_dir}/googletest/include")

project (ZenBenchTests)

set (CMAKE_CXX_STANDARD 11)

include_directories(../include ${GTEST_INCLUDES})
add_executable (tests Context_tests.cpp)
target_link_libraries (tests libgtest libgtest_main)