/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2022, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#pragma once

#include "utils/network/TcpConnection.hpp"

#include <osal/Semaphore.hpp>
#include <osal/Thread.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <system_error>
#include <vector>

namespace utils::network {

/// Function that should be called in a separate thread by TcpServer in order to handle incoming connection.
/// @param connection           Helper object allowing reading and writing from/to the remote endpoint.
using TcpConnectionHandler = std::function<void(TcpConnection connection)>;

/// Represents TCP/IP server allowing to handle single or multiple clients in parallel.
class TcpServer {
public:
    /// Constructor.
    /// @param port                     Port on which TCP server should listen.
    /// @param maxConnections           Maximal number of concurrent connections (number of spawned threads).
    /// @param maxPendingConnections    Maximal number of pending connections in the kernels queue.
    explicit TcpServer(int port = m_cUninitialized,
                       unsigned int maxConnections = m_cDefaultMaxConnections,
                       unsigned int maxPendingConnections = m_cDefaultMaxPendingConnections);

    /// Copy constructor.
    /// @note This constructor is deleted, because TcpServer is not meant to be copy-constructed.
    TcpServer(const TcpServer&) = delete;

    /// Move constructor.
    /// @param other                    Object to be moved from.
    TcpServer(TcpServer&& other) noexcept;

    /// Destructor. Automatically stops the server if it is still running during destruction.
    ~TcpServer();

    /// Copy assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpServer is not meant to be copy-assigned.
    TcpServer& operator=(const TcpServer&) = delete;

    /// Move assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpServer is not meant to be move-assigned.
    TcpServer& operator=(TcpServer&&) = delete;

    /// Registers user-defined connection handler that should be launched in a separate thread to handle new connection.
    /// @param connectionHandler        Function to be invoked.
    /// @return Flag indicating if this operation was successful.
    /// @param true                     Handler has been registered.
    /// @param false                    Handler has not been registered
    bool setConnectionHandler(TcpConnectionHandler connectionHandler);

    /// Returns flag indicating if server has been started.
    /// @return Flag indicating if server has been started.
    /// @retval true                    Sever has been started.
    /// @retval false                   Server has not been started.
    [[nodiscard]] bool isRunning() const { return m_running; }

    /// Starts TCP server. After call to this method a listening thread will be spawned and clients will be able to
    /// connected.
    /// @return Error code of the operation.
    /// @note This call is non-blocking.
    /// @note This method assumes, that port has already been set in server's constructor.
    std::error_code start();

    /// Starts TCP server. After call to this method a listening thread will be spawned and clients will be able to
    /// connected.
    /// @param port                     Port on which TCP server should listen.
    /// @return Error code of the operation.
    /// @note This call is non-blocking.
    /// @note This method assumes, that port has already been set in server's constructor.
    std::error_code start(int port);

    /// Stops TCP server.
    /// @note This method can block until all server threads are finished.
    void stop();

private:
    /// Thread function that listens for incoming connections and spawns connection-specific threads according to
    /// server's settings.
    void listenThread();

    /// Wrapper for the user-defined connection thread.
    /// @param connection               Connection object that should be passed to the user handler.
    void connectionThread(TcpConnection connection);

    /// Closes the underlying network socket.
    void closeSocket();

private:
    static constexpr int m_cUninitialized = -1;
    static constexpr unsigned int m_cDefaultMaxConnections = 1;
    static constexpr unsigned int m_cDefaultMaxPendingConnections = 10;
    static constexpr unsigned int m_cListenThreadStackSize = 128 * 1024;
    static constexpr unsigned int m_cConnectionThreadStackSize = 128 * 1024;

    bool m_running{};
    int m_port;
    unsigned int m_maxPendingConnections;
    int m_socket{m_cUninitialized};
    TcpConnectionHandler m_connectionHandler;
    osal::Semaphore m_startSemaphore{0};
    osal::Semaphore m_connectionsSemaphore;
    osal::NormalPrioThread<m_cListenThreadStackSize> m_listenThread;
    std::vector<osal::NormalPrioThread<m_cConnectionThreadStackSize>> m_connectionThreads;
};

} // namespace utils::network
