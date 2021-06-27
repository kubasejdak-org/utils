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

TcpConnection::TcpConnection(int socket,
                             Endpoint localEndpoint,
                             Endpoint remoteEndpoint,
                             OptionalReferenceFlag parentRunning)
    : m_socket(socket)
    , m_localEndpoint(std::move(localEndpoint))
    , m_remoteEndpoint(std::move(remoteEndpoint))
    , m_cParentRunning(parentRunning)
{
    TcpConnectionLogger::info("Created TCP/IP network connection with the following parameters:");
    TcpConnectionLogger::info("  local endpoint IP    : {}", m_localEndpoint.ip);
    TcpConnectionLogger::info("  local endpoint port  : {}", m_localEndpoint.port);
    if (m_localEndpoint.name)
        TcpConnectionLogger::info("  local endpoint name  : {}", *m_localEndpoint.name);
    TcpConnectionLogger::info("  remote endpoint IP   : {}", m_remoteEndpoint.ip);
    TcpConnectionLogger::info("  remote endpoint port : {}", m_remoteEndpoint.port);
    if (m_remoteEndpoint.name)
        TcpConnectionLogger::info("  remote endpoint name : {}", *m_remoteEndpoint.name);
}

TcpConnection::TcpConnection(TcpConnection&& other) noexcept
    : m_socket(std::exchange(other.m_socket, m_cUninitialized))
    , m_localEndpoint(std::move(other.m_localEndpoint))
    , m_remoteEndpoint(std::move(other.m_remoteEndpoint))
    , m_cParentRunning(other.m_cParentRunning)
{}

TcpConnection::~TcpConnection()
{
    close();
}

std::error_code TcpConnection::read(BytesVector& bytes, std::size_t size, osal::Timeout timeout)
{
    bytes.resize(size);
    if (bytes.size() != size) {
        TcpConnectionLogger::error("read: Failed to resize vector");
        return Error::eNoMemory;
    }

    std::size_t actualReadSize{};
    auto error = read(bytes.data(), size, actualReadSize, timeout);
    bytes.resize(actualReadSize);
    return error;
}

std::error_code
TcpConnection::read(std::uint8_t* bytes, std::size_t size, std::size_t& actualReadSize, osal::Timeout timeout)
{
    if (!isActive()) {
        TcpConnectionLogger::error("read: Connection is not active");
        close();
        return Error::eConnectionNotActive;
    }

    if (bytes == nullptr) {
        TcpConnectionLogger::error("read: Bytes pointer is nullptr");
        return Error::eInvalidArgument;
    }

    actualReadSize = 0;

    while (isActive()) {
        if (timeout.isExpired()) {
            TcpConnectionLogger::debug("Read timeout occurred: {} ms", durationMs(timeout));
            return Error::eTimeout;
        }

        fd_set dataReadFds{};
        FD_ZERO(&dataReadFds);          // NOLINT
        FD_SET(m_socket, &dataReadFds); // NOLINT
        constexpr int cTimeoutMs = 250;
        timeval dataTimeout{0, int(osalMsToUs(cTimeoutMs))};

        TcpConnectionLogger::trace("Waiting for endpoint data");
        if (select(m_socket + 1, &dataReadFds, nullptr, nullptr, &dataTimeout) > 0) {
            auto bytesCount = ::read(m_socket, bytes, size);
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
            actualReadSize = bytesCount;
            return Error::eOk;
        }
    }

    close();
    return Error::eConnectionNotActive;
}

std::error_code TcpConnection::write(std::string_view text)
{
    return write(reinterpret_cast<const uint8_t*>(text.data()), text.size()); // NOLINT
}

std::error_code TcpConnection::write(const BytesVector& bytes)
{
    return write(bytes.data(), bytes.size());
}

std::error_code TcpConnection::write(const std::uint8_t* bytes, std::size_t size)
{
    if (!isActive()) {
        TcpConnectionLogger::error("write: Connection is not active");
        close();
        return Error::eConnectionNotActive;
    }

    if (bytes == nullptr) {
        TcpConnectionLogger::error("write: Bytes pointer is nullptr");
        return Error::eInvalidArgument;
    }

    if (size == 0)
        return Error::eOk;

    auto toWrite = size;
    while (toWrite != 0) {
        auto bytesCount = ::write(m_socket, bytes, size);
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
        ::close(m_socket);
        m_socket = m_cUninitialized;
    }
}

} // namespace utils::network
