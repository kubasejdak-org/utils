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

#include <osal/Semaphore.hpp>
#include <osal/Thread.hpp>
#include <osal/sleep.hpp>
#include <utils/network/Error.hpp>
#include <utils/network/TcpClient.hpp>
#include <utils/network/TcpServer.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

template <typename T = std::size_t>
T generateRandomNumber(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
{
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<T> distribution(min, max);

    return distribution(generator);
}

void generateRandomData(std::size_t size, utils::network::BytesVector& bytes)
{
    bytes.resize(size);
    std::generate(bytes.begin(), bytes.end(), [] { return generateRandomNumber<std::uint8_t>(); });
}

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

TEST_CASE("2. Simple server echo test", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isParentRunning() && connection.isActive()) {
            if (auto error = connection.read(bytes, cMaxSize, 100ms)) {
                if (error == utils::network::Error::eTimeout)
                    continue;

                break;
            }

            if (connection.write(bytes))
                break;
        }
    });

    auto error = server.start();
    REQUIRE(!error);

    osal::sleep(10ms);

    utils::network::TcpClient client("localhost", cPort);
    error = client.connect();
    REQUIRE(!error);

    osal::sleep(5ms);

    std::size_t toSend{};

    SECTION("2.1. Send 0 B.") { toSend = 0; }

    SECTION("2.2. Send 1 B.") { toSend = 1; }

    SECTION("2.3. Send 4 KB.")
    {
        constexpr std::size_t cToSend = 4 * 1024;
        toSend = cToSend;
    }

    SECTION("2.4. Send 897987 B.")
    {
        constexpr std::size_t cToSend = 897987;
        toSend = cToSend;
    }

    while (toSend != 0) {
        auto size = std::min(toSend, generateRandomNumber<std::size_t>(0, cMaxSize));
        std::vector<std::uint8_t> writeBytes;
        generateRandomData(size, writeBytes);

        error = client.write(writeBytes);
        REQUIRE(!error);

        std::vector<std::uint8_t> readBytes;
        error = client.read(readBytes, size);
        REQUIRE(!error);
        REQUIRE(readBytes == writeBytes);

        toSend -= size;
    }

    osal::sleep(10ms);
    client.disconnect();
}

struct ThreadSynchro {
    osal::Semaphore subject{0};
    osal::Semaphore manager{0};

    void waitForManagerApproval()
    {
        auto error = subject.wait();
        REQUIRE(!error);
        error = manager.signal();
        REQUIRE(!error);
    }

    void allowSubjectToWork()
    {
        auto error = subject.signal();
        REQUIRE(!error);
        error = manager.timedWait(100ms);
        REQUIRE(!error);
    }

    void notifyManagerOnExit()
    {
        auto error = manager.signal();
        REQUIRE(!error);
    }

    void waitForSubjectExit()
    {
        auto error = manager.timedWait(1s);
        REQUIRE(!error);
    }
};

TEST_CASE("3. Client disconnects from server", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;
    ThreadSynchro synchro;
    std::error_code serverError{};

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isParentRunning() && connection.isActive()) {
            synchro.waitForManagerApproval();

            serverError = connection.read(bytes, cMaxSize, 500ms);
            if (serverError) {
                if (serverError == utils::network::Error::eTimeout)
                    continue;

                break;
            }

            synchro.waitForManagerApproval();

            serverError = connection.write(bytes);
            if (serverError)
                break;

            synchro.waitForManagerApproval();
        }

        synchro.notifyManagerOnExit();
    });

    auto error = server.start();
    REQUIRE(!error);

    utils::network::TcpClient client("localhost", cPort);
    error = client.connect();
    REQUIRE(!error);

    osal::sleep(100ms);

    SECTION("3.1. Server is about to call read()")
    {
        client.disconnect();

        // Allow server to call read().
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(serverError == utils::network::Error::eRemoteEndpointDisconnected);
    }

    SECTION("3.2. Server is blocked on read()")
    {
        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(100ms);
        client.disconnect();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(serverError == utils::network::Error::eRemoteEndpointDisconnected);
    }

    SECTION("3.3. Server is about to call write()")
    {
        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(500ms);
        client.disconnect();

        // Allow server to call write().
        synchro.allowSubjectToWork();

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Allow server to call read().
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(serverError == utils::network::Error::eRemoteEndpointDisconnected);
    }
}

TEST_CASE("4. Server disconnects from client", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;
    ThreadSynchro synchro;

    utils::network::TcpServer server(cPort);

    SECTION("4.1. Client is about to call write()")
    {
        server.setConnectionHandler([&](utils::network::TcpConnection connection) {
            std::vector<std::uint8_t> bytes;
            while (connection.isParentRunning() && connection.isActive()) {
                connection.close();

                // Allow client to call write().
                synchro.allowSubjectToWork();

                // Allow client to call read().
                synchro.allowSubjectToWork();
            }
        });
    }

    SECTION("4.2. Client is about to call read()")
    {
        server.setConnectionHandler([&](utils::network::TcpConnection connection) {
            std::vector<std::uint8_t> bytes;
            while (connection.isParentRunning() && connection.isActive()) {
                // Allow client to call write().
                synchro.allowSubjectToWork();

                if (auto error = connection.read(bytes, cMaxSize, 500ms)) {
                    if (error == utils::network::Error::eTimeout)
                        continue;

                    break;
                }

                connection.close();

                // Allow client to call read().
                synchro.allowSubjectToWork();
            }
        });
    }

    SECTION("4.3. Client is blocked on read()")
    {
        server.setConnectionHandler([&](utils::network::TcpConnection connection) {
            std::vector<std::uint8_t> bytes;
            while (connection.isParentRunning() && connection.isActive()) {
                // Allow client to call write().
                synchro.allowSubjectToWork();

                if (auto error = connection.read(bytes, cMaxSize, 500ms)) {
                    if (error == utils::network::Error::eTimeout)
                        continue;

                    break;
                }

                // Allow client to call read().
                synchro.allowSubjectToWork();

                osal::sleep(50ms);
                connection.close();
            }
        });
    }

    auto error = server.start();
    REQUIRE(!error);

    for (int i = 0; i < 3; ++i) {
        utils::network::TcpClient client("localhost", cPort);
        error = client.connect();
        REQUIRE(!error);

        synchro.waitForManagerApproval();

        // Use different versions of TcpClient::write() and TcpClient::read().
        std::vector<std::uint8_t> writeBytes{1, 2, 3};
        std::vector<std::uint8_t> readBytes;
        readBytes.reserve(cMaxSize);
        std::size_t actualReadSize{};

        switch (i) {
            case 0:
                error = client.write(writeBytes);
                REQUIRE(!error);

                synchro.waitForManagerApproval();

                error = client.read(readBytes, cMaxSize, 100ms);
                break;
            case 1:
                error = client.write(writeBytes.data(), writeBytes.size());
                REQUIRE(!error);

                synchro.waitForManagerApproval();

                error = client.read(readBytes.data(), cMaxSize, actualReadSize, 100ms);
                break;
            case 2:
                error = client.write("Hello world");
                REQUIRE(!error);

                synchro.waitForManagerApproval();

                error = client.read(readBytes, cMaxSize, 100ms);
                break;
            default: break;
        }

        REQUIRE(error == utils::network::Error::eRemoteEndpointDisconnected);
    }
}

TEST_CASE("5. Server is stopped while connection is active, check server", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;
    ThreadSynchro synchro;
    std::error_code serverError{};

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isParentRunning() && connection.isActive()) {
            synchro.waitForManagerApproval();

            serverError = connection.read(bytes, cMaxSize, 500ms);
            if (serverError) {
                if (serverError == utils::network::Error::eTimeout)
                    continue;

                break;
            }

            synchro.waitForManagerApproval();

            serverError = connection.write(bytes);
            if (serverError)
                break;

            synchro.waitForManagerApproval();
        }

        synchro.notifyManagerOnExit();
    });

    auto error = server.start();
    REQUIRE(!error);

    utils::network::TcpClient client("localhost", cPort);
    error = client.connect();
    REQUIRE(!error);

    osal::sleep(100ms);

    SECTION("5.1. Server is about to call read()")
    {
        error = client.write("Hello world");
        REQUIRE(!error);

        osal::Thread<> stopThread([&] { server.stop(); });

        // Allow server to call read().
        synchro.allowSubjectToWork();

        // Allow server to call write().
        synchro.allowSubjectToWork();

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }

    SECTION("5.2. Server is blocked on read()")
    {
        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(100ms);
        osal::Thread<> stopThread([&] { server.stop(); });

        osal::sleep(100ms);
        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call write().
        synchro.allowSubjectToWork();

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }

    SECTION("5.3. Server is about to call write()")
    {
        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(100ms);
        osal::Thread<> stopThread([&] { server.stop(); });
        osal::sleep(100ms);

        // Allow server to call write().
        synchro.allowSubjectToWork();

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }
}

TEST_CASE("6. Server is stopped while connection is active, check client", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;
    ThreadSynchro synchro;
    std::error_code serverError{};

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isParentRunning() && connection.isActive()) {
            synchro.waitForManagerApproval();

            serverError = connection.read(bytes, cMaxSize, 500ms);
            if (serverError) {
                if (serverError == utils::network::Error::eTimeout)
                    continue;

                break;
            }

            synchro.waitForManagerApproval();

            serverError = connection.write(bytes);
            if (serverError)
                break;

            synchro.waitForManagerApproval();
        }

        synchro.notifyManagerOnExit();
    });

    auto error = server.start();
    REQUIRE(!error);

    utils::network::TcpClient client;
    error = client.connect("localhost", cPort);
    REQUIRE(!error);

    osal::sleep(100ms);

    SECTION("6.1. Client is about to call write()")
    {
        osal::Thread<> stopThread([&] { server.stop(); });
        osal::sleep(100ms);

        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call read().
        synchro.allowSubjectToWork();

        // Allow server to call write().
        synchro.allowSubjectToWork();

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }

    SECTION("6.2. Client is about to call read()")
    {
        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(100ms);
        osal::Thread<> stopThread([&] { server.stop(); });
        osal::sleep(100ms);

        // Allow server to call write().
        synchro.allowSubjectToWork();

        std::vector<std::uint8_t> bytes;
        error = client.read(bytes, cMaxSize, 500ms);
        REQUIRE(!error);

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }

    SECTION("6.3. Client is blocked on read()")
    {
        error = client.write("Hello world");
        REQUIRE(!error);

        // Allow server to call read().
        synchro.allowSubjectToWork();

        osal::sleep(100ms);
        osal::Thread<> helperThread([&] {
            osal::sleep(100ms);
            osal::Thread<> stopThread([&] { server.stop(); });
            osal::sleep(100ms);

            // Allow server to call write().
            synchro.allowSubjectToWork();

            server.stop();
        });

        std::vector<std::uint8_t> bytes;
        error = client.read(bytes, cMaxSize, 500ms);
        REQUIRE(!error);

        // Allow server to start next iteration.
        synchro.allowSubjectToWork();

        // Wait for server to close connection.
        synchro.waitForSubjectExit();
        REQUIRE(!serverError);
    }
}

TEST_CASE("7. Performing operations in incorrect connection state", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 256;
    ThreadSynchro synchro;

    utils::network::TcpServer server(cPort);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        auto error = connection.read(bytes, cMaxSize, 100ms);
        REQUIRE(error == utils::network::Error::eTimeout);

        std::size_t actualReadSize{};
        error = connection.read(nullptr, cMaxSize, actualReadSize, 100ms);
        REQUIRE(error == utils::network::Error::eInvalidArgument);

        error = connection.write(nullptr, cMaxSize);
        REQUIRE(error == utils::network::Error::eInvalidArgument);

        connection.close();

        error = connection.read(bytes, cMaxSize, 100ms);
        REQUIRE(error == utils::network::Error::eConnectionNotActive);

        error = connection.write("Hello world");
        REQUIRE(error == utils::network::Error::eConnectionNotActive);
    });

    auto error = server.start();
    REQUIRE(!error);

    utils::network::TcpClient client("localhost", cPort);
    error = client.connect();
    REQUIRE(!error);

    osal::sleep(500ms);
}

TEST_CASE("8. Multiple simple server echo test", "[unit][TcpConnection]")
{
    constexpr int cPort = 10101;
    constexpr std::size_t cMaxSize = 255;

    constexpr unsigned int cClientsCount = 10;
    utils::network::TcpServer server(cPort, cClientsCount);
    server.setConnectionHandler([&](utils::network::TcpConnection connection) {
        std::vector<std::uint8_t> bytes;
        while (connection.isParentRunning() && connection.isActive()) {
            if (auto error = connection.read(bytes, cMaxSize, 100ms)) {
                if (error == utils::network::Error::eTimeout)
                    continue;

                break;
            }

            if (connection.write(bytes))
                break;
        }
    });

    auto error = server.start();
    REQUIRE(!error);

    osal::sleep(10ms);

    auto clientThread = [&](const std::vector<std::uint8_t>& writeBytes) {
        utils::network::TcpClient client("localhost", cPort);
        auto error = client.connect();
        if (error)
            REQUIRE(!error);

        constexpr int cIterationsCount = 1000;
        for (int i = 0; i < cIterationsCount; ++i) {
            error = client.write(writeBytes);
            if (error)
                REQUIRE(!error);

            std::vector<std::uint8_t> readBytes;
            error = client.read(readBytes, writeBytes.size());
            if (error)
                REQUIRE(!error);
            if (readBytes != writeBytes)
                REQUIRE(readBytes == writeBytes);
        }
    };

    std::array<std::vector<std::uint8_t>, cClientsCount> bytes{};
    for (auto& data : bytes)
        generateRandomData(cMaxSize, data);

    constexpr unsigned int cClientThreadStackSize = 128 * 1024;
    using ClientThread = osal::NormalPrioThread<cClientThreadStackSize>;
    std::array<ClientThread, cClientsCount> clientThreads{ClientThread{clientThread, bytes[0]},
                                                          ClientThread{clientThread, bytes[1]},
                                                          ClientThread{clientThread, bytes[2]},
                                                          ClientThread{clientThread, bytes[3]},
                                                          ClientThread{clientThread, bytes[4]},
                                                          ClientThread{clientThread, bytes[5]},  // NOLINT
                                                          ClientThread{clientThread, bytes[6]},  // NOLINT
                                                          ClientThread{clientThread, bytes[7]},  // NOLINT
                                                          ClientThread{clientThread, bytes[8]},  // NOLINT
                                                          ClientThread{clientThread, bytes[9]}}; // NOLINT
}
