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

#pragma once

#include "utils/network/TcpConnection.hpp"

#include <osal/Semaphore.hpp>
#include <osal/Thread.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace utils::network {

using ClientCallback = std::function<void(const Endpoint& endpoint)>;
using ConnectionCallback = std::function<void(TcpConnection connection)>;

class TcpServer {
public:
    explicit TcpServer(int port = m_cUninitialized,
                       int maxConnections = m_cDefaultMaxConnections,
                       int maxPendingConnections = m_cDefaultMaxPendingConnections);
    TcpServer(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = default;
    ~TcpServer();
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer& operator=(TcpServer&&) = delete;

    bool setOnConnectedCallback(ClientCallback callback);
    bool setOnDisconnectedCallback(ClientCallback callback);
    bool setConnectionCallback(ConnectionCallback callback);

    bool start();
    bool start(int port);
    void stop();

private:
    void listenThread();
    void connectionThread(TcpConnection connection);
    void closeSocket();

private:
    static constexpr int m_cUninitialized = -1;
    static constexpr int m_cDefaultMaxConnections = 1;
    static constexpr int m_cDefaultMaxPendingConnections = 10;

    bool m_running{};
    int m_port;
    int m_maxConnections;
    int m_maxPendingConnections;
    int m_socket{m_cUninitialized};
    ClientCallback m_onClientConnected;
    ClientCallback m_onClientDisconnected;
    ConnectionCallback m_connectionCallback;
    osal::Semaphore m_startSemaphore{0};
    osal::Semaphore m_connectionsSemaphore;
    osal::Thread<> m_listenThread;
    std::vector<osal::Thread<>> m_connectionThreads;
};

} // namespace utils::network
