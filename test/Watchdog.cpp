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
#include <osal/timestamp.hpp>
#include <utils/watchdog/Watchdog.hpp>

#include <catch2/catch.hpp>
#include <fmt/printf.h>

#include <chrono>
#include <string>

struct Client {
    std::chrono::milliseconds timeout{};
    bool expired{};
    osal::Timestamp end;
};

TEST_CASE("1. Timeouts without resetting", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;

    SECTION("1.1. Timeout 300 ms") { timeout = 300ms; }

    SECTION("1.2. Timeout 10 ms") { timeout = 10ms; }

    SECTION("1.3. Timeout 3 s") { timeout = 3s; }

    Client clientData{};
    std::string timedOutClient;
    auto timeoutHandler = [&](std::string_view client) {
        clientData.expired = true;
        clientData.end = osal::timestamp();
        timedOutClient = client;
    };

    utils::watchdog::Watchdog watchdog("TestWdg");
    std::string client = "test1";
    watchdog.registerClient(client, timeoutHandler, timeout);

    auto start = osal::timestamp();
    watchdog.start();
    osal::sleep(timeout + 1ms);
    watchdog.stop();

    fmt::print("end - start : {}\n", (clientData.end - start).count());

    REQUIRE(clientData.expired);
    REQUIRE((clientData.end - start) >= timeout);
    REQUIRE((clientData.end - start) <= (timeout + 1ms));
    REQUIRE(timedOutClient == client);
}

TEST_CASE("2. Multiple identical timeouts without resetting", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;

    SECTION("2.1. Timeout 300 ms") { timeout = 300ms; }

    SECTION("2.2. Timeout 10 ms") { timeout = 10ms; }

    SECTION("2.3. Timeout 3 s") { timeout = 3s; }

    std::map<std::string, Client> clientData
        = {{"test1", {timeout, false, {}}}, {"test2", {timeout, false, {}}}, {"test3", {timeout, false, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].expired = true;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& client : clientData) {
        watchdog.registerClient(client.first, timeoutHandler, timeout);
    }

    auto start = osal::timestamp();
    watchdog.start();
    osal::sleep(timeout + 1ms);
    watchdog.stop();

    for (const auto& [name, data] : clientData) {
        fmt::print("{}: end - start : {}\n", name, (data.end - start).count());

        REQUIRE(data.expired);
        REQUIRE((data.end - start) >= timeout);
        REQUIRE((data.end - start) <= (timeout + 1ms));
    }
}
