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

#include "utils/network/TcpConnection.hpp"

#include "utils/network/Error.hpp"
#include "utils/network/logger.hpp"

#include <osal/timestamp.h>
#include <sys/select.h>
#include <unistd.h>

#include <utility>

namespace utils::network {

TcpConnection::TcpConnection(const bool& serverRunning, int socket, Endpoint endpoint)
    : m_serverRunning(serverRunning)
    , m_socket(socket)
    , m_endpoint(std::move(endpoint))
{
    TcpConnectionLogger::info("Created TCP/IP connection with the following parameters:");
    TcpConnectionLogger::info("  endpoint IP   : {}", m_endpoint.ip);
    if (endpoint.name)
        TcpConnectionLogger::info("  endpoint name : {}", *m_endpoint.name);
}

TcpConnection::TcpConnection(TcpConnection&& other) noexcept
    : m_serverRunning(other.m_serverRunning)
    , m_socket(std::exchange(other.m_socket, m_cUninitialized))
    , m_endpoint(std::move(other.m_endpoint))
{}

TcpConnection::~TcpConnection()
{
    close();
}

std::error_code TcpConnection::read(BytesVector& bytes, std::size_t size, osal::Timeout timeout)
{
    if (!isActive()) {
        TcpConnectionLogger::error("Connection is not active");
        close();
        return Error::eConnectionNotActive;
    }

    while (m_serverRunning) {
        if (timeout.isExpired()) {
            TcpConnectionLogger::debug("Read timeout occurred: {} ms", durationMs(timeout));
            close();
            return Error::eTimeout;
        }

        fd_set dataReadFds{};
        FD_ZERO(&dataReadFds);          // NOLINT
        FD_SET(m_socket, &dataReadFds); // NOLINT
        timeval dataTimeout{0, int(osalMsToUs(100))};

        TcpConnectionLogger::trace("Waiting for endpoint data");
        if (select(m_socket + 1, &dataReadFds, nullptr, nullptr, &dataTimeout) > 0) {
            bytes.resize(size);
            auto bytesCount = ::read(m_socket, bytes.data(), bytes.size());
            if (bytesCount == 0) {
                TcpConnectionLogger::info("Endpoint disconnected, closing connection");
                close();
                return Error::eClientDisconnected;
            }
            if (bytesCount == -1) {
                TcpConnectionLogger::warn("read() returned error: {}", strerror(errno));
                if (errno != EAGAIN) {
                    TcpConnectionLogger::error("Closing connection on error");
                    break;
                }

                continue;
            }

            TcpConnectionLogger::trace("Read {} bytes", bytesCount);
            bytes.resize(bytesCount);
            return Error::eOk;
        }
    }

    close();
    return Error::eServerStopped;
}

std::error_code TcpConnection::write(const std::vector<std::uint8_t>& bytes)
{
    if (!isActive()) {
        TcpConnectionLogger::error("Connection is not active");
        close();
        return Error::eConnectionNotActive;
    }

    auto toWrite = bytes.size();
    while (toWrite != 0) {
        auto bytesCount = ::write(m_socket, bytes.data(), bytes.size());
        if (bytesCount == 0) {
            TcpConnectionLogger::warn("write() returned 0");
            return Error::eWriteError;
        }
        if (bytesCount == -1) {
            TcpConnectionLogger::warn("write() returned error: {}", strerror(errno));
            if (errno != EAGAIN) {
                TcpConnectionLogger::error("Closing connection on error");
                break;
            }

            continue;
        }

        toWrite -= bytesCount;
    }

    return Error::eOk;
}

void TcpConnection::close()
{
    if (m_socket != m_cUninitialized) {
        TcpConnectionLogger::info("STOP");
        ::close(m_socket);
        m_socket = m_cUninitialized;
    }
}

} // namespace utils::network
