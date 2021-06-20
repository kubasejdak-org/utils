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

#include <utils/network/TcpServer.hpp>

#include <catch2/catch.hpp>

TEST_CASE("1. Creating TCP/IP server multiple time on the same port", "[unit][TcpServer]")
{
    constexpr int cIterationsCount = 10;
    constexpr int cPort = 10101;

    SECTION("1.1. Uninitialized server")
    {
        for (int i = 0; i < cIterationsCount; ++i) {
            utils::network::TcpServer server;
            REQUIRE(!server.isRunning());
        }
    }

    SECTION("1.2. Port initialized server")
    {
        for (int i = 0; i < cIterationsCount; ++i) {
            utils::network::TcpServer server(cPort);
            REQUIRE(!server.isRunning());
        }
    }

    SECTION("1.3. Fully initialized server")
    {
        constexpr int cMaxConnections = 6;
        constexpr int cMaxPendingConnections = 20;
        for (int i = 0; i < cIterationsCount; ++i) {
            utils::network::TcpServer server(cPort, cMaxConnections, cMaxPendingConnections);
            REQUIRE(!server.isRunning());
        }
    }
}

TEST_CASE("2. Starting TCP/IP server multiple times on the same port", "[unit][TcpServer]")
{
    constexpr int cIterationsCount = 10;
    constexpr int cPort = 10101;

    SECTION("2.1. Uninitialized server")
    {
        for (int i = 0; i < cIterationsCount; ++i) {
            utils::network::TcpServer server;
            REQUIRE(!server.isRunning());

            auto error = server.start(cPort);
            REQUIRE(!error);
            REQUIRE(server.isRunning());

            server.stop();
            REQUIRE(!server.isRunning());
        }
    }

    SECTION("2.2. Initialized server")
    {
        for (int i = 0; i < cIterationsCount; ++i) {
            utils::network::TcpServer server;
            REQUIRE(!server.isRunning());

            auto error = server.start(cPort);
            REQUIRE(!error);
            REQUIRE(server.isRunning());

            server.stop();
            REQUIRE(!server.isRunning());
        }
    }
}
