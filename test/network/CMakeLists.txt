target_sources(utils-tests PRIVATE
    Error.cpp
    TcpClient.cpp
    TcpConnection.cpp
    TcpServer.cpp
    types.cpp
)

target_link_libraries(utils-tests
    PRIVATE utils::network
)
