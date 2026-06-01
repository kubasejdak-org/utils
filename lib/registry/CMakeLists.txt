add_library(utils-registry INTERFACE)
add_library(utils::registry ALIAS utils-registry)

target_include_directories(utils-registry
    INTERFACE
        include
)

find_package(osal)

target_link_libraries(utils-registry
    INTERFACE
        osal::cpp
)
