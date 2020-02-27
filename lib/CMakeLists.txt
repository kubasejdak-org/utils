cmake_minimum_required(VERSION 3.15)

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)

add_library(utils INTERFACE)

target_include_directories(
    INTERFACE include
)
