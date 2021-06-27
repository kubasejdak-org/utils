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

#include <utils/network/types.hpp>

#include <catch2/catch.hpp>

TEST_CASE("1. Invalid data given to getLocalEndpoint()", "[unit][TcpConnection]")
{
    auto endpoint = utils::network::getLocalEndpoint(-1);
    REQUIRE(endpoint.ip.empty());
    REQUIRE(endpoint.port == 0);
    REQUIRE(!endpoint.name);
}

TEST_CASE("2. Checking if IP is valid", "[unit][TcpConnection]")
{
    REQUIRE(utils::network::isValidIp("127.0.0.1"));
    REQUIRE(utils::network::isValidIp("192.168.0.15"));

    REQUIRE(!utils::network::isValidIp("localhost"));
    REQUIRE(!utils::network::isValidIp("Hello world"));
    REQUIRE(!utils::network::isValidIp("localhost.168.1.16"));
    REQUIRE(!utils::network::isValidIp("-1.-1.-1.-1"));
}

TEST_CASE("3. Resolving address to IP", "[unit][TcpConnection]")
{
    REQUIRE(utils::network::addressToIp("127.0.0.1") == "127.0.0.1");
    REQUIRE(utils::network::addressToIp("192.168.0.15") == "192.168.0.15");
    REQUIRE(utils::network::addressToIp("localhost") == "127.0.0.1");
    REQUIRE(utils::network::addressToIp("Hello world").empty());
    REQUIRE(utils::network::addressToIp("localhost.168.1.16").empty());
    REQUIRE(utils::network::addressToIp("-1.-1.-1.-1").empty());
}
