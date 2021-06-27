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

#include <osal/sleep.hpp>
#include <utils/network/Error.hpp>
#include <utils/network/TcpClient.hpp>
#include <utils/network/TcpServer.hpp>

#include <catch2/catch.hpp>

TEST_CASE("1. Connect and disconnect from server", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    bool connected{};
    utils::network::Endpoint serverLocalEndpoint{};
    utils::network::Endpoint serverRemoteEndpoint{};

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        connected = true;
        serverLocalEndpoint = connection.localEndpoint();
        serverRemoteEndpoint = connection.remoteEndpoint();

        while (connection.isParentRunning() && connection.isActive())
            osal::sleep(1ms);
    });

    auto error = server.start();
    REQUIRE(!error);

    osal::sleep(10ms);

    utils::network::TcpClient client("localhost", cPort);
    error = client.connect();
    REQUIRE(!error);

    osal::sleep(5ms);
    REQUIRE(connected);

    auto clientLocalEndpoint = client.localEndpoint();
    auto clientRemoteEndpoint = client.remoteEndpoint();
    REQUIRE(clientLocalEndpoint.ip == "127.0.0.1");
    REQUIRE(clientRemoteEndpoint.ip == "127.0.0.1");
    REQUIRE(*clientLocalEndpoint.name == "localhost");
    REQUIRE(*clientRemoteEndpoint.name == "localhost");
    REQUIRE(clientLocalEndpoint.ip == serverRemoteEndpoint.ip);
    REQUIRE(clientRemoteEndpoint.ip == serverLocalEndpoint.ip);
    REQUIRE(clientLocalEndpoint.port == serverRemoteEndpoint.port);
    REQUIRE(clientRemoteEndpoint.port == serverLocalEndpoint.port);
    REQUIRE(clientLocalEndpoint.name == serverRemoteEndpoint.name);
    REQUIRE(clientRemoteEndpoint.name == serverLocalEndpoint.name);

    client.disconnect();
}
