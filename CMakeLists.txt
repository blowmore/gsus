#  minimum required cmake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)

project(gsus)

SET(CMAKE_BUILD_TYPE Release)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror -g3 -ggdb)

set(DEPENDENCIES -lpthread)

add_executable(gsus src/main.cpp)
