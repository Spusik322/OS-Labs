cmake_minimum_required(VERSION 3.10)
project(Lab1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(Number STATIC NumberLibrary.cpp)
set_target_properties(Number PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(Vector SHARED VectorLibrary.cpp)
target_link_libraries(Vector PRIVATE Number)
target_include_directories(Vector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(client NumberVectorClient.cpp)
target_link_libraries(client PRIVATE Vector Number)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})