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
#include "utils/network/types.hpp"

#include <osal/Timeout.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <system_error>

namespace utils::network {

class TcpClient {
public:
    TcpClient();
    TcpClient(std::string address, int port);
    TcpClient(const TcpClient&) = delete;
    TcpClient(TcpClient&& other) noexcept;
    ~TcpClient();
    TcpClient& operator=(const TcpClient&) = delete;
    TcpClient& operator=(TcpClient&&) = delete;

    std::error_code connect();
    std::error_code connect(std::string_view address, int port);
    void disconnect();

    [[nodiscard]] Endpoint localEndpoint() const;
    [[nodiscard]] Endpoint remoteEndpoint() const;

    std::error_code read(BytesVector& bytes, std::size_t size, osal::Timeout timeout = osal::Timeout::infinity());
    std::error_code read(std::uint8_t* bytes, std::size_t size, osal::Timeout timeout, std::size_t& actualReadSize);
    std::error_code write(const BytesVector& bytes);
    std::error_code write(const std::uint8_t* bytes, std::size_t size);

private:
    static constexpr int m_cUninitialized = -1;

    bool m_running{};
    std::string m_address;
    int m_port;
    std::unique_ptr<TcpConnection> m_connection;
};

} // namespace utils::network
