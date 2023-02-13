cmake_minimum_required(VERSION 3.24)

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++20> $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)

add_subdirectory(bits)
add_subdirectory(fsm)
add_subdirectory(functional)
add_subdirectory(info)
add_subdirectory(logger)
add_subdirectory(preprocessor)
add_subdirectory(registry)
add_subdirectory(types)
add_subdirectory(watchdog)

if (OSAL_PLATFORM STREQUAL linux)
    add_subdirectory(network)
endif ()
