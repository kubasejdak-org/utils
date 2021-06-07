/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#include "utils/network/TcpServer.hpp"

#include "utils/network/Error.hpp"
#include "utils/network/logger.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <osal/timestamp.h>
#include <sys/socket.h>

#include <array>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <utility>

namespace utils::network {

TcpServer::TcpServer(int port, int maxConnections, int maxPendingConnections)
    : m_port(port)
    , m_maxConnections(maxConnections)
    , m_maxPendingConnections(maxPendingConnections)
    , m_connectionsSemaphore(m_maxConnections)
{
    if (m_maxConnections < 1) {
        TcpServerLogger::critical("Failed to create TCP/IP server: maxConnections cannot be less than 1");
        assert(false);
    }

    if (m_maxPendingConnections < 0) {
        TcpServerLogger::critical("Failed to create TCP/IP server: maxPendingConnections cannot be less than 0");
        assert(false);
    }

    TcpServerLogger::info("Created TCP/IP server with the following parameters:");
    TcpServerLogger::info("  max connections         : {}", m_maxConnections);
    TcpServerLogger::info("  max pending connections : {}", m_maxPendingConnections);
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::setConnectionHandler(TcpConnectionHandler connectionHandler)
{
    if (m_running)
        return false;

    m_connectionHandler = std::move(connectionHandler);
    return true;
}

std::error_code TcpServer::start()
{
    return start(m_cUninitialized);
}

std::error_code TcpServer::start(int port)
{
    if (m_running) {
        TcpServerLogger::error("Failed to start: server is already started");
        return Error::eServerRunning;
    }

    if (port != m_cUninitialized)
        m_port = port;

    if (m_port == m_cUninitialized) {
        TcpServerLogger::error("Failed to start TCP/IP server: uninitialized port value ({})", m_port);
        return Error::eInvalidArgument;
    }

    TcpServerLogger::info("Starting TCP/IP server with the following parameters:");
    TcpServerLogger::info("  port : {}", m_port);

    assert(m_socket == -1);
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1) {
        TcpServerLogger::error("Failed to create AF_INET socket: {}", strerror(errno));
        return Error::eSocketError;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) { // NOLINT
        TcpServerLogger::error("Failed to bind socket to address: {}", strerror(errno));
        closeSocket();
        return Error::eBindError;
    }

    listen(m_socket, m_maxPendingConnections);

    if (auto error = m_listenThread.start([this] { listenThread(); })) {
        TcpServerLogger::error("Failed to start listening thread: err={}", error.message());
        closeSocket();
        return error;
    }

    constexpr auto cStartupTimeout = 1s;
    if (m_startSemaphore.timedWait(cStartupTimeout)) {
        TcpServerLogger::error("Timeout in listening thread startup");
        return Error::eTimeout;
    }

    m_running = true;
    return Error::eOk;
}

void TcpServer::stop()
{
    m_running = false;
    m_listenThread.join();
    m_connectionThreads.clear();
}

void TcpServer::listenThread()
{
    m_running = true;
    m_startSemaphore.signal();

    TcpServerLogger::info("Listening thread started");

    while (m_running) {
        constexpr auto cTimeout = 250ms;
        if (m_connectionsSemaphore.timedWait(cTimeout))
            continue;

        TcpServerLogger::trace("Waiting for TCP client");

        while (m_running) {
            fd_set clientReadFds{};
            FD_ZERO(&clientReadFds);          // NOLINT
            FD_SET(m_socket, &clientReadFds); // NOLINT
            timeval clientTimeout{0, int(osalMsToUs(cTimeout.count()))};

            if (select(m_socket + 1, &clientReadFds, nullptr, nullptr, &clientTimeout) > 0) {
                TcpServerLogger::debug("Incoming TCP connection");

                sockaddr_in clientAddr{};
                socklen_t size = sizeof(clientAddr);

                // NOLINTNEXTLINE
                auto clientSocket = accept4(m_socket, reinterpret_cast<sockaddr*>(&clientAddr), &size, SOCK_CLOEXEC);
                m_connectionThreads.emplace_back([&] {
                    Endpoint localEndpoint{};
                    {
                        sockaddr_in serverAddr{};
                        socklen_t serverSize = sizeof(serverAddr);

                        // NOLINTNEXTLINE
                        if (getsockname(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), &serverSize) == -1) {
                            TcpServerLogger::error("getsockname() returned error for local endpoint: err={}",
                                                   strerror(errno));
                        }

                        localEndpoint.ip = inet_ntoa(serverAddr.sin_addr);
                        localEndpoint.port = serverAddr.sin_port;

                        std::array<char, NI_MAXHOST> name{};

                        // NOLINTNEXTLINE
                        if (getnameinfo(reinterpret_cast<sockaddr*>(&clientAddr),
                                        size,
                                        name.data(),
                                        name.size(),
                                        nullptr,
                                        0,
                                        NI_NAMEREQD)
                            == 0)
                            localEndpoint.name = std::string(name.begin(), name.end());
                    }

                    Endpoint remoteEndpoint{};
                    {
                        remoteEndpoint.ip = inet_ntoa(clientAddr.sin_addr);
                        remoteEndpoint.port = clientAddr.sin_port;

                        std::array<char, NI_MAXHOST> name{};

                        // NOLINTNEXTLINE
                        if (getnameinfo(reinterpret_cast<sockaddr*>(&clientAddr),
                                        size,
                                        name.data(),
                                        name.size(),
                                        nullptr,
                                        0,
                                        NI_NAMEREQD)
                            == 0)
                            remoteEndpoint.name = std::string(name.begin(), name.end());
                    }

                    TcpConnection connection(m_running, clientSocket, localEndpoint, remoteEndpoint);
                    connectionThread(std::move(connection));
                });
                break;
            }
        }
    }

    closeSocket();
    TcpServerLogger::info("Listening thread stopped");
}

void TcpServer::connectionThread(TcpConnection connection)
{
    auto endpoint = connection.remoteEndpoint();
    TcpServerLogger::debug("Starting connection thread: remote endpoint ip={}", endpoint.ip);

    m_connectionHandler(std::move(connection));

    TcpServerLogger::debug("Connection thread stopped");
    m_connectionsSemaphore.signal();
}

void TcpServer::closeSocket()
{
    if (m_socket != m_cUninitialized) {
        close(m_socket);
        m_socket = m_cUninitialized;
    }
}

} // namespace utils::network
