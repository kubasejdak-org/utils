add_library(utils-network EXCLUDE_FROM_ALL
    TcpServer.cpp
)
add_library(utils::network ALIAS utils-network)

target_include_directories(utils-network
    PUBLIC include
)

target_link_libraries(utils-network
    PRIVATE osal::cpp
)
