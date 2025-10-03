cmake_minimum_required(VERSION 3.10)
project(Thread)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(thread thread.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(thread PRIVATE -O2)
endif()