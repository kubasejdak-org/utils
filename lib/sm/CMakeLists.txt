add_library(utils-sm INTERFACE)
add_library(utils::sm ALIAS utils-sm)

target_include_directories(utils-sm
    INTERFACE
        include
)

find_package(osal)
find_package(utils COMPONENTS functional logger)

target_link_libraries(utils-sm
    INTERFACE
        osal::cpp
        utils::functional
        utils::logger
)
