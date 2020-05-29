add_library(utils-property INTERFACE)
add_library(utils::property ALIAS utils-property)

target_include_directories(utils-property
    INTERFACE include
)
