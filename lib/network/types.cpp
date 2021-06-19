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

#include "utils/network/types.hpp"

#include "utils/network/logger.hpp"

#include <arpa/inet.h>
#include <netdb.h>

#include <array>
#include <cerrno>
#include <cstring>

namespace utils::network {

/// Creates and returns Endpoint out of given sockaddr_in.
/// @param addr             sockaddr_in to be used.
/// @return Endpoint created out of given sockaddr_in object.
static Endpoint addrToEndpoint(const sockaddr_in& addr)
{
    socklen_t addrSize = sizeof(addr);

    Endpoint endpoint{};
    endpoint.ip = inet_ntoa(addr.sin_addr);
    endpoint.port = addr.sin_port;

    std::array<char, NI_MAXHOST> name{};

    if (getnameinfo(reinterpret_cast<const sockaddr*>(&addr), // NOLINT
                    addrSize,
                    name.data(),
                    name.size(),
                    nullptr,
                    0,
                    NI_NAMEREQD)
        == 0)
        endpoint.name = std::string(name.begin(), name.end());

    return endpoint;
}

Endpoint getLocalEndpoint(int socket)
{
    sockaddr_in addr{};
    socklen_t addrSize = sizeof(addr);

    if (getsockname(socket, reinterpret_cast<sockaddr*>(&addr), &addrSize) == -1) { // NOLINT
        NetworkTypesLogger::error("getsockname() returned error for local endpoint: err={}", strerror(errno));
        return {};
    }

    return addrToEndpoint(addr);
}

Endpoint getRemoteEndpoint(const sockaddr_in& addr)
{
    return addrToEndpoint(addr);
}

} // namespace utils::network
