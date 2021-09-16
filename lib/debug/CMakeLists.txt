add_library(utils-debug EXCLUDE_FROM_ALL
    EventStats.cpp
)
add_library(utils::debug ALIAS utils-debug)

target_include_directories(utils-debug
    PUBLIC include
)

target_link_libraries(utils-debug
    PUBLIC osal::cpp
    PRIVATE utils::logger
)
