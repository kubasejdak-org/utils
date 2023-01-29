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

#include <utils/network/Error.hpp>
#include <utils/network/TcpClient.hpp>
#include <utils/network/TcpServer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

TEST_CASE("1. Create TcpClient", "[unit][TcpClient]")
{
    SECTION("1.1. Client is uninitialized")
    {
        utils::network::TcpClient client;
    }

    SECTION("1.2. Client is initialized")
    {
        constexpr int cPort = 10101;
        utils::network::TcpClient client("localhost", cPort);
    }
}

TEST_CASE("2. Moving TcpClient around", "[unit][TcpClient]")
{
    constexpr int cPort = 10101;
    utils::network::TcpServer server(cPort);
    auto error = server.start();
    REQUIRE(!error);

    utils::network::TcpClient client1("localhost", cPort);
    utils::network::TcpClient client2(std::move(client1));
    error = client2.connect();
    REQUIRE(!error);
}

TEST_CASE("3. Performing operations in incorrect TcpClient state", "[unit][TcpClient]")
{
    constexpr int cPort = 10101;
    utils::network::TcpClient client("localhost", cPort);

    SECTION("3.1. No remote endpoint")
    {
        auto error = client.connect();
        REQUIRE(error == utils::network::Error::eConnectError);
    }

    SECTION("3.2. Client is already running")
    {
        utils::network::TcpServer server(cPort);
        auto error = server.start();
        REQUIRE(!error);

        error = client.connect();
        REQUIRE(!error);

        error = client.connect();
        REQUIRE(error == utils::network::Error::eClientRunning);
    }

    SECTION("3.3. Bad endpoint address")
    {
        auto error = client.connect("badAddress", cPort);
        REQUIRE(error == utils::network::Error::eInvalidArgument);
    }

    SECTION("3.4. Getting local/remote endpoints when client is not connected")
    {
        auto localEndpoint = client.localEndpoint();
        REQUIRE(localEndpoint.ip.empty());
        REQUIRE(localEndpoint.port == 0);
        REQUIRE(!localEndpoint.name);

        auto remoteEndpoint = client.remoteEndpoint();
        REQUIRE(remoteEndpoint.ip.empty());
        REQUIRE(remoteEndpoint.port == 0);
        REQUIRE(!remoteEndpoint.name);
    }

    SECTION("3.5. Reading when client is not connected")
    {
        constexpr std::size_t cSize = 15;
        std::vector<std::uint8_t> bytes;
        auto error = client.read(bytes, cSize);
        REQUIRE(error == utils::network::Error::eClientDisconnected);

        bytes.reserve(cSize);
        std::size_t actualReadSize{};
        error = client.read(bytes.data(), cSize, actualReadSize);
        REQUIRE(error == utils::network::Error::eClientDisconnected);
    }

    SECTION("3.6. Writing when client is not connected")
    {
        std::vector<std::uint8_t> bytes{1, 2, 3};
        auto error = client.write({1, 2, 3});
        REQUIRE(error == utils::network::Error::eClientDisconnected);

        error = client.write(bytes.data(), bytes.size());
        REQUIRE(error == utils::network::Error::eClientDisconnected);

        error = client.write("Hello world");
        REQUIRE(error == utils::network::Error::eClientDisconnected);
    }
}
