cmake_minimum_required(VERSION 3.10)
project(Channel)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(channel channel.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(channel PRIVATE -O2)
endif()