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

#include "utils/network/types.hpp"

#include <osal/Timeout.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <system_error>

namespace utils::network {

/// Represents a TCP/IP network connection. This class should be used in user handlers to communicate with
/// remote endpoint and control connection state on demand.
class TcpConnection {
public:
    /// Type representing optional reference boolean flag.
    using OptionalReferenceFlag = std::optional<std::reference_wrapper<bool>>;

    /// Constructor.
    /// @param socket               Socket, which is used in current connection.
    /// @param localEndpoint        Description of the local endpoint.
    /// @param remoteEndpoint       Description of the remote endpoint.
    /// @param parentRunning        Optional reference to flag indicating if parent object is still running.
    TcpConnection(int socket,
                  Endpoint localEndpoint,
                  Endpoint remoteEndpoint,
                  OptionalReferenceFlag parentRunning = {});

    /// Copy constructor.
    /// @note This constructor is deleted, because TcpConnection is not meant to be copy-constructed.
    TcpConnection(const TcpConnection&) = delete;

    /// Move constructor.
    /// @param other                Object to be moved from.
    TcpConnection(TcpConnection&& other) noexcept;

    /// Destructor.
    ~TcpConnection();

    /// Copy assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpConnection is not meant to be copy-assigned.
    TcpConnection& operator=(const TcpConnection&) = delete;

    /// Move assignment operator.
    /// @return Reference to self.
    /// @note This operator is deleted, because TcpConnection is not meant to be move-assigned.
    TcpConnection& operator=(TcpConnection&&) = delete;

    /// Returns object representing local endpoint (this side of the connection).
    /// @return Object representing local endpoint (this side of the connection).
    [[nodiscard]] Endpoint localEndpoint() const { return m_localEndpoint; };

    /// Returns object representing remote endpoint (other side of the connection).
    /// @return Object representing remote endpoint (other side of the connection).
    [[nodiscard]] Endpoint remoteEndpoint() const { return m_remoteEndpoint; };

    /// Returns flag indicating if parent (if any exists) is running.
    /// @return Flag indicating if parent (if any exists) is running.
    /// @note This method makes sense only for TcpServer side.
    [[nodiscard]] bool isParentRunning() const { return m_cParentRunning && *m_cParentRunning; }

    /// Returns flag indicating if this connection is still active. If not, then this object can and should be removed.
    /// @return Flag indicating if this connection is still active.
    [[nodiscard]] bool isActive() const { return m_socket != m_cUninitialized; }

    /// Receives demanded number of bytes from the remote endpoint associated with this connection.
    /// @param bytes                Vector where the received data will be placed by this method.
    /// @param size                 Number of bytes to be received from the remote endpoint.
    /// @param timeout              Maximal time to wait for the data.
    /// @return Error code of the operation.
    /// @note This method does not assume, that the output vector has the proper capacity. It will be
    ///       automatically expanded, if needed, by the container itself. Size of the vector after call
    ///       to this method will indicate the actual number of read bytes.
    std::error_code read(BytesVector& bytes, std::size_t size, osal::Timeout timeout = osal::Timeout::infinity());

    /// Receives demanded number of bytes from the remote endpoint associated with this connection.
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

    /// Sends given string to the remote endpoint associated with this connection.
    /// @param text                 String to be sent.
    /// @return Error code of the operation.
    std::error_code write(std::string_view text);

    /// Sends given vector of bytes to the remote endpoint associated with this connection.
    /// @param bytes                Vector of raw bytes to be sent.
    /// @return Error code of the operation.
    std::error_code write(const BytesVector& bytes);

    /// Sends given memory block of bytes to the remote endpoint associated with this connection.
    /// @param bytes                Memory block of raw bytes to be sent.
    /// @param size                 Size of the memory block to be sent.
    /// @return Error code of the operation.
    std::error_code write(const std::uint8_t* bytes, std::size_t size);

    /// Closes the connection.
    /// @note After call to this method Connection object is unusable and should be removed.
    void close();

private:
    static constexpr int m_cUninitialized = -1;

    int m_socket;
    Endpoint m_localEndpoint;
    Endpoint m_remoteEndpoint;
    const OptionalReferenceFlag m_cParentRunning;
};

} // namespace utils::network
