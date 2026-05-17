add_library(utils-logger INTERFACE)
add_library(utils::logger ALIAS utils-logger)

target_include_directories(utils-logger
    INTERFACE
        include
)

find_package(spdlog)

target_link_libraries(utils-logger
    INTERFACE
        spdlog::spdlog
)
