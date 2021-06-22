add_library(utils-preprocessor INTERFACE)
add_library(utils::preprocessor ALIAS utils-preprocessor)

target_include_directories(utils-preprocessor
    INTERFACE include
)
