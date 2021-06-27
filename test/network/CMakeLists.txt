target_sources(apputils-tests
    PRIVATE Error.cpp
            TcpClient.cpp
            TcpConnection.cpp
            TcpServer.cpp
            types.cpp
)

target_link_libraries(apputils-tests
    PRIVATE utils::network
)
