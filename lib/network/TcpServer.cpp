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

#include "utils/network/logger.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <osal/timestamp.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <cassert>
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

bool TcpServer::setOnConnectedCallback(ClientCallback callback)
{
    if (m_onClientConnected)
        return false;

    m_onClientConnected = std::move(callback);
    return true;
}

bool TcpServer::setOnDisconnectedCallback(ClientCallback callback)
{
    if (m_onClientDisconnected)
        return false;

    m_onClientDisconnected = std::move(callback);
    return true;
}

bool TcpServer::setOnDataCallback(DataCallback callback)
{
    if (m_onDataReceived)
        return false;

    m_onDataReceived = std::move(callback);
    return true;
}

bool TcpServer::start()
{
    return start(m_cUninitialized);
}

bool TcpServer::start(int port)
{
    if (m_running) {
        TcpServerLogger::error("Failed to start: server is already started");
        return false;
    }

    if (port != m_cUninitialized)
        m_port = port;

    if (m_port == m_cUninitialized) {
        TcpServerLogger::error("Failed to start TCP/IP server: uninitialized port value ({})", m_port);
        return false;
    }

    TcpServerLogger::info("Starting TCP/IP server with the following parameters:");
    TcpServerLogger::info("  port : {}", m_port);

    assert(m_socket == -1);
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1) {
        TcpServerLogger::error("Failed to create AF_INET socket: {}", strerror(errno));
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) { // NOLINT
        TcpServerLogger::error("Failed to bind socket to address: {}", strerror(errno));
        closeSocket();
        return false;
    }

    listen(m_socket, m_maxPendingConnections);

    if (auto error = m_listenThread.start([this] { listenThread(); })) {
        TcpServerLogger::error("Failed to start connection thread: err={}", error.message());
        closeSocket();
    }

    constexpr auto cStartupTimeout = 1s;
    if (m_startSemaphore.timedWait(cStartupTimeout)) {
        TcpServerLogger::error("Timeout in connection thread startup");
        m_running = false;
    }

    return m_running;
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

    TcpServerLogger::info("Connection thread started");

    while (m_running) {
        if (m_connectionsSemaphore.timedWait(250ms))
            continue;

        TcpServerLogger::trace("Waiting for TCP client");

        while (m_running) {
            fd_set clientReadFds{};
            FD_ZERO(&clientReadFds);          // NOLINT
            FD_SET(m_socket, &clientReadFds); // NOLINT
            timeval clientTimeout{0, int(osalMsToUs(1))};

            if (select(m_socket + 1, &clientReadFds, nullptr, nullptr, &clientTimeout) > 0) {
                TcpServerLogger::debug("Incoming TCP connection");

                sockaddr_in clientAddr{};
                socklen_t size = sizeof(clientAddr);

                // NOLINTNEXTLINE
                auto clientSocket = accept4(m_socket, reinterpret_cast<sockaddr*>(&clientAddr), &size, SOCK_CLOEXEC);
                std::string clientIp = inet_ntoa(clientAddr.sin_addr);
                m_connectionThreads.emplace_back([&] { connectionThread(clientIp, clientSocket); });
                break;
            }
        }
    }

    closeSocket();
    TcpServerLogger::info("Connection thread stopped");
}

void TcpServer::connectionThread(std::string clientIp, int clientSocket)
{
    if (m_onClientConnected)
        m_onClientConnected(clientIp);

    TcpServerLogger::debug("Starting connection thread: clientIp={}, clientSocket={}", clientIp, clientSocket);

    while (m_running) {
        fd_set dataReadFds{};
        FD_ZERO(&dataReadFds);              // NOLINT
        FD_SET(clientSocket, &dataReadFds); // NOLINT
        timeval dataTimeout{0, int(osalMsToUs(250))};

        TcpServerLogger::trace("Waiting for client data");
        if (select(clientSocket + 1, &dataReadFds, nullptr, nullptr, &dataTimeout) > 0) {
            constexpr int cBufferSize = 256;
            std::vector<std::uint8_t> buffer(cBufferSize);
            auto bytesCount = read(clientSocket, buffer.data(), buffer.size());
            if (bytesCount == 0) {
                TcpServerLogger::info("Client disconnected, closing connection");
                break;
            }
            if (bytesCount == -1) {
                TcpServerLogger::warn("Read returned error: {}", strerror(errno));
                if (errno != EAGAIN) {
                    TcpServerLogger::error("Closing connection on error");
                    break;
                }

                continue;
            }

            TcpServerLogger::trace("Read {} bytes", bytesCount);

            buffer.resize(bytesCount);
            if (m_onDataReceived)
                m_onDataReceived(clientIp, std::move(buffer));
        }
    }

    close(clientSocket);
    TcpServerLogger::debug("Connection closed");

    if (m_onClientDisconnected)
        m_onClientDisconnected(clientIp);

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
