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

#include <osal/Timeout.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace utils::network {

/// Represents set of information about network endpoint (local or remote).
/// @note This class will usually be used for debugging purposes as well as client filtering.
struct Endpoint {
    std::string ip;
    int port;
    std::optional<std::string> name;
};

/// Represents a network connection. This class should be used in user handlers to communicate with remote endpoint and
/// control connection state on demand.
class Connection {
public:
    /// Helper type alias representing vector of bytes.
    using BytesVector = std::vector<std::uint8_t>;

    /// Constructor.
    /// @param serverRunning        Reference to flag indicating if parent network server is still running.
    /// @param socket               Socket, which is used in current connection.
    /// @param localEndpoint        Description of the local endpoint.
    /// @param remoteEndpoint       Description of the remote endpoint.
    Connection(const bool& serverRunning, int socket, Endpoint localEndpoint, Endpoint remoteEndpoint);

    /// Copy constructor.
    /// @note This constructor is deleted, because Connection is not meant to be copy-constructed.
    Connection(const Connection&) = delete;

    /// Move constructor.
    /// @param other                Object to be moved from.
    Connection(Connection&& other) noexcept;

    /// Destructor.
    ~Connection();

    /// Copy assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because Connection is not meant to be copy-assigned.
    Connection& operator=(const Connection&) = delete;

    /// Move assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because Connection is not meant to be move-assigned.
    Connection& operator=(Connection&&) = delete;

    /// Returns object representing local endpoint (this side of the connection).
    /// @return Object representing local endpoint (this side of the connection).
    [[nodiscard]] Endpoint localEndpoint() const { return m_localEndpoint; };

    /// Returns object representing remote endpoint (other side of the connection).
    /// @return Object representing remote endpoint (other side of the connection).
    [[nodiscard]] Endpoint remoteEndpoint() const { return m_remoteEndpoint; };
    [[nodiscard]] bool isActive() const { return m_serverRunning && (m_socket != m_cUninitialized); }

    std::error_code read(BytesVector& bytes, std::size_t size, osal::Timeout timeout = osal::Timeout::infinity());
    [[nodiscard]] std::error_code write(const std::vector<std::uint8_t>& bytes);
    void close();

private:
    static constexpr int m_cUninitialized = -1;

    const bool& m_serverRunning;
    int m_socket;
    Endpoint m_localEndpoint;
    Endpoint m_remoteEndpoint;
};

} // namespace utils::network
