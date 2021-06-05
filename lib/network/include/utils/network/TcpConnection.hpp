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

struct Endpoint {
    std::string ip;
    int port;
    std::optional<std::string> name;
};

class TcpConnection {
public:
    /// Helper type alias representing vector of bytes.
    using BytesVector = std::vector<std::uint8_t>;

    TcpConnection(const bool& serverRunning, int socket, Endpoint localEndpoint, Endpoint remoteEndpoint);
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection(TcpConnection&& other) noexcept;
    ~TcpConnection();
    TcpConnection& operator=(const TcpConnection&) = delete;
    TcpConnection& operator=(TcpConnection&&) = delete;

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
