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
#include <utils/network/Connection.hpp>
#include <utils/network/TcpServer.hpp>

#include <catch2/catch.hpp>
#include <fmt/printf.h>

#include <cstdio>
#include <string>

TEST_CASE("1. Tests", "[unit][TcpServer]")
{
    constexpr int cPort = 10101;
    utils::network::TcpServer server(cPort);
    auto result = server.setConnectionHandler([](utils::network::Connection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isActive()) {
            constexpr std::size_t cSize = 255;
            if (auto error = connection.read(bytes, cSize)) {
                fmt::print("Read error: {}\n", error.message());
                break;
            }

            fmt::print("Received: {}", std::string(bytes.begin(), bytes.end()));
            if (auto error = connection.write(bytes)) {
                fmt::print("Write error: {}\n", error.message());
                break;
            }
        }
    });
    REQUIRE(result);

    result = server.start();
    REQUIRE(result);

    osal::sleep(20s);
    server.stop();
}
