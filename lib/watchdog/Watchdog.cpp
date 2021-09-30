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

#include "utils/watchdog/Watchdog.hpp"

#include "utils/watchdog/logger.hpp"

#include <osal/Error.hpp>
#include <osal/ScopedLock.hpp>

#include <algorithm>

namespace utils::watchdog {

Watchdog::Watchdog(std::string_view name)
    : m_name(name)
{
    WatchdogLogger::info("<{}> Created watchdog", m_name);
}

bool Watchdog::registerClient(std::string_view clientName, WatchdogCallback callback, osal::Timeout timeout)
{
    osal::ScopedLock lock(m_mutex);

    if (m_running) {
        WatchdogLogger::error("<{}> Watchdog is already started", m_name);
        return false;
    }

    if (m_clients.find(clientName.data()) != m_clients.end()) {
        WatchdogLogger::error("<{}> Client already registered: name={}", m_name, clientName);
        return false;
    }

    auto durationMs = osal::durationMs(timeout);
    WatchdogLogger::info("<{}> Registering watchdog client: name={}, timeout={} ms", m_name, clientName, durationMs);
    m_clients.emplace(clientName, ClientData{std::move(callback), timeout});
    return true;
}

bool Watchdog::start()
{
    osal::ScopedLock lock(m_mutex);

    if (m_running) {
        WatchdogLogger::error("<{}> Watchdog is already started", m_name);
        return false;
    }

    if (m_clients.empty()) {
        WatchdogLogger::error("<{}> Watchdog has no registered clients", m_name);
        return false;
    }

    for (auto& clientData : m_clients)
        clientData.second.timeout.reset();

    m_thread = std::make_unique<osal::NormalPrioThread<m_cWatchdogThreadStackSize>>();
    if (auto error = m_thread->start([this] { threadFunc(); })) {
        WatchdogLogger::error("Failed to start watchdog thread: err={}", error.message());
        return false;
    }

    constexpr auto cStartupTimeout = 1s;
    if (m_startSemaphore.timedWait(cStartupTimeout)) {
        WatchdogLogger::error("Timeout in watchdog thread startup");
        return false;
    }

    return true;
}

bool Watchdog::stop()
{
    osal::ScopedLock lock(m_mutex);

    if (!m_running) {
        WatchdogLogger::error("<{}> Watchdog is not started", m_name);
        return false;
    }

    m_running = false;
    m_semaphore.signal();
    m_thread->join();
    m_thread.reset();
    WatchdogLogger::info("<{}> Watchdog stopped", m_name);
    return true;
}

bool Watchdog::reset(std::string_view clientName)
{
    osal::ScopedLock lock(m_mutex);

    if (!m_running) {
        WatchdogLogger::error("<{}> Watchdog is not started", m_name);
        return false;
    }

    auto clientIt = m_clients.find(clientName.data());
    if (clientIt == m_clients.end()) {
        WatchdogLogger::error("<{}> Client not registered: name={}", m_name, clientName);
        return false;
    }

    WatchdogLogger::debug("<{}> Resetting watchdog: client={}", m_name, clientName);
    clientIt->second.timeout.reset();
    m_semaphore.signal();
    return true;
}

void Watchdog::threadFunc()
{
    m_running = true;
    m_startSemaphore.signal();

    WatchdogLogger::info("<{}> Watchdog started", m_name);

    auto timeoutComparator
        = [](const auto& a, const auto& b) { return a.second.timeout.timeLeft() < b.second.timeout.timeLeft(); };

    auto getSmallestTimeout = [&] {
        auto client = std::min_element(m_clients.begin(), m_clients.end(), timeoutComparator);
        return client->second.timeout;
    };

    auto getExpiredClient = [&] {
        return std::find_if(m_clients.begin(), m_clients.end(), [](const auto& client) {
            return client.second.timeout.isExpired();
        });
    };

    while (m_running) {
        auto timeout = getSmallestTimeout();
        if (m_semaphore.timedWait(timeout) == OsalError::eTimeout) {
            auto client = getExpiredClient();
            if (client == m_clients.end()) {
                WatchdogLogger::warn("<{}> Timeout almost occurred: reset() called after semaphore timed-out", m_name);
                continue;
            }

            auto clientName = client->first;
            WatchdogLogger::info("<{}> Timeout occurred: client={}", m_name, clientName);
            client->second.callback(clientName);
            client->second.timeout.reset();
        }
    }
}

} // namespace utils::watchdog
