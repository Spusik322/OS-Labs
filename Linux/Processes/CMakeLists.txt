cmake_minimum_required(VERSION 3.15)
project(ProcessKillerDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -std=c++17)

add_executable(killer killer/main.cpp)
add_executable(user user/main.cpp)

target_link_libraries(killer PRIVATE pthread)
target_link_libraries(user PRIVATE pthread)

set_target_properties(killer user PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
