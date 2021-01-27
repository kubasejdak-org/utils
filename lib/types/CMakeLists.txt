add_library(utils-types INTERFACE)
add_library(utils::types ALIAS utils-types)

target_include_directories(utils-types
    INTERFACE include
)
