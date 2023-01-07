add_library(utils-fsm INTERFACE)
add_library(utils::fsm ALIAS utils-fsm)

target_include_directories(utils-fsm
    INTERFACE include
)

target_link_libraries(utils-fsm
    INTERFACE osal::cpp utils::functional utils::logger
)
