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

#include <osal/Thread.hpp>
#include <osal/sleep.hpp>
#include <osal/timestamp.hpp>
#include <utils/watchdog/Watchdog.hpp>

#include <catch2/catch.hpp>
#include <fmt/printf.h>

#include <chrono>
#include <string>

struct Client {
    std::chrono::milliseconds timeout{};
    int timeoutCounter{};
    osal::Timestamp end;
};

TEST_CASE("1. Calling watchdog functions in wrong state", "[unit][Watchdog]")
{
    utils::watchdog::Watchdog watchdog("TestWdg");

    SECTION("1.1. Watchdog is not started")
    {
        REQUIRE(!watchdog.stop());
        REQUIRE(!watchdog.reset("client1"));
    }

    SECTION("1.2. Watchdog is already started")
    {
        REQUIRE(watchdog.registerClient(
            "client1",
            [](std::string_view /*unused*/) {},
            1s));
        REQUIRE(watchdog.start());

        REQUIRE(!watchdog.start());
        REQUIRE(!watchdog.registerClient(
            "client2",
            [](std::string_view /*unused*/) {},
            1s));

        REQUIRE(watchdog.stop());
    }

    SECTION("1.3. Watchdog has no clients") { REQUIRE(!watchdog.start()); }

    SECTION("1.4. Client is already registered")
    {
        REQUIRE(watchdog.registerClient(
            "client1",
            [](std::string_view /*unused*/) {},
            1s));
        REQUIRE(!watchdog.registerClient(
            "client1",
            [](std::string_view /*unused*/) {},
            1s));
    }

    SECTION("1.5. No such client")
    {
        REQUIRE(watchdog.registerClient(
            "client1",
            [](std::string_view /*unused*/) {},
            1s));
        REQUIRE(watchdog.start());

        REQUIRE(!watchdog.reset("client2"));

        REQUIRE(watchdog.stop());
    }
}

TEST_CASE("2. Start and stop watchdog multiple times", "[unit][Watchdog]")
{
    utils::watchdog::Watchdog watchdog("TestWdg");
    std::string client = "test1";
    REQUIRE(watchdog.registerClient(
        client,
        [](std::string_view /*unused*/) {},
        1s));

    constexpr int cIterationsCount = 100;
    for (int i = 0; i < cIterationsCount; ++i) {
        REQUIRE(watchdog.start());
        REQUIRE(watchdog.stop());
    }
}

TEST_CASE("3. Timeouts without resetting", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;
    int iterationsCount{};

    SECTION("3.1. Timeout 300 ms")
    {
        timeout = 300ms;
        constexpr int cIterationsCount = 100;
        iterationsCount = cIterationsCount;
    }

    SECTION("3.2. Timeout 100 ms")
    {
        timeout = 100ms;
        constexpr int cIterationsCount = 100;
        iterationsCount = cIterationsCount;
    }

    SECTION("3.3. Timeout 3 s")
    {
        timeout = 3s;
        constexpr int cIterationsCount = 10;
        iterationsCount = cIterationsCount;
    }

    Client clientData{};
    std::string timedOutClient;
    auto timeoutHandler = [&](std::string_view client) {
        clientData.timeoutCounter++;
        clientData.end = osal::timestamp();
        timedOutClient = client;
    };

    utils::watchdog::Watchdog watchdog("TestWdg");
    std::string client = "test1";
    REQUIRE(watchdog.registerClient(client, timeoutHandler, timeout));

    for (int i = 0; i < iterationsCount; ++i) {
        clientData = Client();
        auto start = osal::timestamp();
        REQUIRE(watchdog.start());
        osal::sleep(timeout + 2ms);
        REQUIRE(watchdog.stop());

        auto elapsed = clientData.end - start;
        fmt::print("end - start : {}\n", elapsed.count());

        REQUIRE(clientData.timeoutCounter == 1);
        REQUIRE(elapsed >= timeout);
        REQUIRE(elapsed <= (timeout + 2ms));
        REQUIRE(timedOutClient == client);
    }
}

TEST_CASE("4. Multiple identical timeouts without resetting", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;

    SECTION("4.1. Timeout 300 ms") { timeout = 300ms; }

    SECTION("4.2. Timeout 100 ms") { timeout = 100ms; }

    SECTION("4.3. Timeout 3 s") { timeout = 3s; }

    std::map<std::string, Client> clientData
        = {{"test1", {timeout, 0, {}}}, {"test2", {timeout, 0, {}}}, {"test3", {timeout, 0, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].timeoutCounter++;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& [name, _] : clientData)
        REQUIRE(watchdog.registerClient(name, timeoutHandler, timeout));

    auto start = osal::timestamp();
    REQUIRE(watchdog.start());
    osal::sleep(timeout + 2ms);
    REQUIRE(watchdog.stop());

    for (const auto& [name, data] : clientData) {
        auto elapsed = data.end - start;
        fmt::print("{}: end - start : {}\n", name, elapsed.count());

        REQUIRE(data.timeoutCounter == 1);
        REQUIRE(elapsed >= timeout);
        REQUIRE(elapsed <= (timeout + 2ms));
    }
}

TEST_CASE("5. Resetting single watchdog before timeout", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;

    SECTION("5.1. Timeout 300 ms") { timeout = 300ms; }

    SECTION("5.2. Timeout 100 ms") { timeout = 100ms; }

    SECTION("5.3. Timeout 3 s") { timeout = 3s; }

    Client clientData{};
    std::string timedOutClient;
    auto timeoutHandler = [&](std::string_view client) {
        clientData.timeoutCounter++;
        clientData.end = osal::timestamp();
        timedOutClient = client;
    };

    utils::watchdog::Watchdog watchdog("TestWdg");
    std::string client = "test1";
    REQUIRE(watchdog.registerClient(client, timeoutHandler, timeout));

    REQUIRE(watchdog.start());
    osal::sleep(timeout / 2);

    REQUIRE(watchdog.reset(client));

    osal::sleep(timeout / 2);
    REQUIRE(watchdog.stop());

    REQUIRE(clientData.timeoutCounter == 0);
}

TEST_CASE("6. Resetting multiple identical watchdogs before timeout", "[unit][Watchdog]")
{
    std::chrono::milliseconds timeout;

    SECTION("6.1. Timeout 300 ms") { timeout = 300ms; }

    SECTION("6.2. Timeout 100 ms") { timeout = 100ms; }

    SECTION("6.3. Timeout 3 s") { timeout = 3s; }

    std::map<std::string, Client> clientData
        = {{"test1", {timeout, 0, {}}}, {"test2", {timeout, 0, {}}}, {"test3", {timeout, 0, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].timeoutCounter++;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& [name, _] : clientData)
        REQUIRE(watchdog.registerClient(name, timeoutHandler, timeout));

    REQUIRE(watchdog.start());
    osal::sleep(timeout / 2);

    for (const auto& [name, _] : clientData)
        REQUIRE(watchdog.reset(name));

    osal::sleep(timeout / 2);
    REQUIRE(watchdog.stop());

    for (const auto& [_, data] : clientData)
        REQUIRE(data.timeoutCounter == 0);
}

TEST_CASE("7. Resetting multiple watchdogs in separate threads before timeout, fixed scenario", "[unit][Watchdog]")
{
    std::map<std::string, Client> clientData
        = {{"test1", {400ms, 0, {}}}, {"test2", {400ms, 0, {}}}, {"test3", {300ms, 0, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].timeoutCounter++;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& [name, data] : clientData)
        REQUIRE(watchdog.registerClient(name, timeoutHandler, data.timeout));

    REQUIRE(watchdog.start());

    osal::Thread<> thread1([&] {
        const auto* name = "test1";

        osal::sleep(398ms);
        watchdog.reset(name);

        osal::sleep(398ms);
        watchdog.reset(name);

        osal::sleep(398ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);

        osal::sleep(200ms);
        watchdog.reset(name);
    });

    osal::Thread<> thread2([&] {
        const auto* name = "test2";

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(398ms);
        watchdog.reset(name);

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);

        osal::sleep(300ms);
        watchdog.reset(name);
    });

    osal::Thread<> thread3([&] {
        const auto* name = "test3";

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(298ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);

        osal::sleep(298ms);
        watchdog.reset(name);

        osal::sleep(200ms);
        watchdog.reset(name);

        osal::sleep(100ms);
        watchdog.reset(name);
    });

    thread1.join();
    thread2.join();
    thread3.join();
    REQUIRE(watchdog.stop());

    for (const auto& [_, data] : clientData)
        REQUIRE(data.timeoutCounter == 0);
}

TEST_CASE("8. Resetting multiple identical watchdogs in separate threads, fixed scenario", "[unit][Watchdog]")
{
    auto timeout = 100ms;
    std::map<std::string, Client> clientData
        = {{"test1", {timeout, 0, {}}}, {"test2", {timeout, 0, {}}}, {"test3", {timeout, 0, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].timeoutCounter++;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& [name, data] : clientData)
        REQUIRE(watchdog.registerClient(name, timeoutHandler, data.timeout));

    REQUIRE(watchdog.start());

    constexpr int cIterationCount = 100;
    osal::Thread<> thread1([&] {
        const auto* name = "test1";

        for (int i = 0; i < cIterationCount; ++i) {
            osal::sleep(timeout - 2ms);
            watchdog.reset(name);
        }
    });

    osal::Thread<> thread2([&] {
        const auto* name = "test2";

        for (int i = 0; i < cIterationCount; ++i) {
            osal::sleep(timeout - 2ms);
            watchdog.reset(name);
        }
    });

    osal::Thread<> thread3([&] {
        const auto* name = "test3";

        for (int i = 0; i < cIterationCount; ++i) {
            osal::sleep(timeout - 2ms);
            watchdog.reset(name);
        }
    });

    thread1.join();
    thread2.join();
    thread3.join();
    REQUIRE(watchdog.stop());

    for (const auto& [_, data] : clientData)
        REQUIRE(data.timeoutCounter == 0);
}

TEST_CASE("9. Multiple watchdogs, resetting only half of them", "[unit][Watchdog]")
{
    auto timeout = 100ms;
    std::map<std::string, Client> clientData = {{"test1", {timeout, 0, {}}},
                                                {"test2", {timeout, 0, {}}},
                                                {"test3", {timeout, 0, {}}},
                                                {"test4", {timeout, 0, {}}}};

    auto timeoutHandler = [&](std::string_view client) {
        clientData[client.data()].timeoutCounter++;
        clientData[client.data()].end = osal::timestamp();
    };

    utils::watchdog::Watchdog watchdog;
    for (const auto& [name, data] : clientData)
        REQUIRE(watchdog.registerClient(name, timeoutHandler, data.timeout));

    REQUIRE(watchdog.start());
    auto start = osal::timestamp();

    constexpr int cIterationCount = 100;
    for (int i = 0; i < cIterationCount; ++i) {
        osal::sleep(timeout - 2ms);
        REQUIRE(watchdog.reset("test1"));
        REQUIRE(watchdog.reset("test3"));
    }

    auto end = osal::timestamp();
    REQUIRE(watchdog.stop());

    auto elapsed = end - start;
    auto expiredCount = elapsed / timeout;
    fmt::print("elapsed      : {} ms\n", elapsed.count());
    fmt::print("expiredCount : {}\n", expiredCount);

    for (const auto& [name, data] : clientData)
        fmt::print("{}: {}\n", name, data.timeoutCounter);

    REQUIRE(clientData["test1"].timeoutCounter == 0);
    REQUIRE(clientData["test2"].timeoutCounter <= expiredCount);
    REQUIRE(clientData["test2"].timeoutCounter >= expiredCount - 1);
    REQUIRE(clientData["test3"].timeoutCounter == 0);
    REQUIRE(clientData["test4"].timeoutCounter <= expiredCount);
    REQUIRE(clientData["test4"].timeoutCounter >= expiredCount - 1);
}
