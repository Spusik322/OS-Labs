cmake_minimum_required(VERSION 3.15)
project(ProcessKillerDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(killer killer/main.cpp)
target_compile_definitions(killer PRIVATE UNICODE _UNICODE)
target_link_libraries(killer PRIVATE Psapi)

add_executable(user user/main.cpp)
target_compile_definitions(user PRIVATE UNICODE _UNICODE)
target_link_libraries(user PRIVATE Psapi)
