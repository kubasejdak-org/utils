cmake_minimum_required(VERSION 3.15)

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a>)

add_subdirectory(logger)
add_subdirectory(property)
add_subdirectory(registry)
