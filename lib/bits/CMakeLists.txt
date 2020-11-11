add_library(utils-bits INTERFACE)
add_library(utils::bits ALIAS utils-bits)

target_include_directories(utils-bits
    INTERFACE include
)
