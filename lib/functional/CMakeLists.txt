add_library(utils-functional INTERFACE)
add_library(utils::functional ALIAS utils-functional)

target_include_directories(utils-functional
    INTERFACE include
)

target_link_libraries(utils-functional
    INTERFACE utils::preprocessor utils::types
)
