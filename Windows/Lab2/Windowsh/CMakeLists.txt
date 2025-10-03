cmake_minimum_required(VERSION 3.10)
project(Windowsh)

# Устанавливаем стандарт C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows добавляем необходимые флаги
if(WIN32)
    # Связываем с Windows API библиотеками
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0600")
endif()

# Создаем исполняемый файл
add_executable(Windowsh Windowsh.cpp)

# Для Windows связываем с необходимыми библиотеками
if(WIN32)
    target_link_libraries(Windowsh kernel32.lib user32.lib)
endif()

# Включаем оптимизацию для Release сборки
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(Windowsh PRIVATE /O2)
endif()