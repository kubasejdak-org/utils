target_sources(apputils-tests
    PRIVATE TcpClient.cpp
            TcpConnection.cpp
            TcpServer.cpp
)

target_link_libraries(apputils-tests
    PRIVATE utils::network
)
