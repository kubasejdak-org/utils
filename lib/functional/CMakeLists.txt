add_library(utils-functional INTERFACE)
add_library(utils::functional ALIAS utils-functional)

target_include_directories(utils-functional
    INTERFACE include
)
