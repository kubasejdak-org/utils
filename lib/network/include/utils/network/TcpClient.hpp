/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
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

/// Represents TCP/IP client allowing to handle one connection with the server.
class TcpClient {
public:
    /// Constructor. Creates na uninitialized TCP/IP client.
    /// @note This constructor requires calling connect(address, port) overload for connecting with remote endpoint.
    TcpClient();

    /// Constructor. Creates na initialized TCP/IP client with given remote server address and port.
    /// @param address          Address of the endpoint to which we want to connect to (can be both IP and hostname).
    /// @param port             Address of the endpoint to which we want to connect to.
    /// @note Even if client is created with this constructor it is allowed to call the connect(address, port) overload
    ///       for connecting with remote endpoint. A typical use case is when you need to change IP/port in runtime.
    TcpClient(std::string address, int port);

    /// Copy constructor.
    /// @note This constructor is deleted, because TcpClient is not meant to be copy-constructed.
    TcpClient(const TcpClient&) = delete;

    /// Move constructor.
    /// @param other            Object to be moved from.
    TcpClient(TcpClient&& other) noexcept;

    /// Destructor. Automatically disconnects if it is still connected during destruction.
    ~TcpClient();

    /// Copy assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpClient is not meant to be copy-assigned.
    TcpClient& operator=(const TcpClient&) = delete;

    /// Move assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpClient is not meant to be move-assigned.
    TcpClient& operator=(TcpClient&&) = delete;

    /// Connects to the remote endpoint using address and port set in proper constructor.
    /// @return Error code of the operation.
    std::error_code connect();

    /// Connects to the remote endpoint using given address and port.
    /// @param address          Address of the endpoint to which we want to connect to (can be both IP and hostname).
    /// @param port             Address of the endpoint to which we want to connect to.
    /// @return Error code of the operation.
    std::error_code connect(std::string_view address, int port);
    void disconnect();

    /// Returns object representing local endpoint (this side of the connection).
    /// @return Object representing local endpoint (this side of the connection).
    [[nodiscard]] Endpoint localEndpoint() const;

    /// Returns object representing remote endpoint (other side of the connection).
    /// @return Object representing remote endpoint (other side of the connection).
    [[nodiscard]] Endpoint remoteEndpoint() const;

    /// Receives demanded number of bytes from the remote endpoint associated with this client.
    /// @param bytes                Vector where the received data will be placed by this method.
    /// @param size                 Number of bytes to be received from the remote endpoint.
    /// @param timeout              Maximal time to wait for the data.
    /// @return Error code of the operation.
    /// @note This method does not assume, that the output vector has the proper capacity. It will be
    ///       automatically expanded, if needed, by the container itself. Size of the vector after call
    ///       to this method will indicate the actual number of read bytes.
    std::error_code read(BytesVector& bytes, std::size_t size, osal::Timeout timeout = osal::Timeout::infinity());

    /// Receives demanded number of bytes from the remote endpoint associated with this client.
    /// @param bytes                Memory block where the received data will be placed by this method.
    /// @param size                 Number of bytes to be received from the remote endpoint.
    /// @param actualReadSize       Actual number of received bytes.
    /// @param timeout              Maximal time to wait for the data.
    /// @return Error code of the operation.
    /// @note This method assumes, that the output memory block has the proper capacity. After call to this
    ///       method the 'actualReadSize' parameter will indicate the actual number of received bytes.
    ///       It is also assumed, that output memory block is empty.
    std::error_code read(std::uint8_t* bytes,
                         std::size_t size,
                         std::size_t& actualReadSize,
                         osal::Timeout timeout = osal::Timeout::infinity());

    /// Sends given string to the remote endpoint associated with this client.
    /// @param text                 String to be sent.
    /// @return Error code of the operation.
    std::error_code write(std::string_view text);

    /// Sends given vector of bytes to the remote endpoint associated with this client.
    /// @param bytes                Vector of raw bytes to be sent.
    /// @return Error code of the operation.
    std::error_code write(const BytesVector& bytes);

    /// Sends given memory block of bytes to the remote endpoint associated with this client.
    /// @param bytes                Memory block of raw bytes to be sent.
    /// @param size                 Size of the memory block to be sent.
    /// @return Error code of the operation.
    std::error_code write(const std::uint8_t* bytes, std::size_t size);

private:
    static constexpr int m_cUninitialized = -1;

    bool m_running{};
    std::string m_address;
    int m_port;
    std::unique_ptr<TcpConnection> m_connection;
};

} // namespace utils::network
