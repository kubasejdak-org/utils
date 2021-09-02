add_library(utils-watchdog EXCLUDE_FROM_ALL
    Error.cpp
    Watchdog.cpp
)
add_library(utils::watchdog ALIAS utils-watchdog)

target_include_directories(utils-watchdog
    PUBLIC include
)

target_link_libraries(utils-watchdog
    PUBLIC osal::cpp
    PRIVATE utils::logger
)
