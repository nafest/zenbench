cmake_minimum_required (VERSION 3.5)
include (ExternalProject)

# get and build gtest
ExternalProject_Add(
    googletest
    DOWNLOAD_COMMAND git clone https://github.com/google/googletest.git
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND ""
)
add_library (libgtest IMPORTED STATIC GLOBAL)
add_library (libgtest_main IMPORTED STATIC GLOBAL)
add_dependencies (libgtest googletest)
add_dependencies (libgtest_main googletest)

ExternalProject_Get_Property (googletest source_dir binary_dir)

if (WIN32)
set (GTEST_LIB gtest.lib)
set (GTEST_MAIN_LIB gtest_main.lib)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
set (GTEST_LIB libgtest.a)
set (GTEST_MAIN_LIB libgtest_main.a)
endif()

set_target_properties (libgtest PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/${GTEST_LIB}"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
   # "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/googletest/include"
)
set_target_properties (libgtest_main PROPERTIES
    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/${GTEST_MAIN_LIB}"
    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
   # "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/googletest/include"
)

set (GTEST_INCLUDES "${source_dir}/googletest/include")

project (ZenBenchTests)
find_package(Threads REQUIRED)

set (CMAKE_CXX_STANDARD 11)

include_directories(../include ${GTEST_INCLUDES})
add_executable (tests Context_tests.cpp)
target_link_libraries (tests libgtest libgtest_main ${CMAKE_THREAD_LIBS_INIT})
