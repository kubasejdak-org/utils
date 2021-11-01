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

#pragma once

#include <osal/Mutex.hpp>
#include <osal/Semaphore.hpp>
#include <osal/Thread.hpp>
#include <osal/Timeout.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace utils::watchdog {

/// Alias for type representing callback when watchdog gets timeout.
using WatchdogCallback = std::function<void(std::string_view)>;

/// Generic software watchdog capable of handling multiple separate clients.
class Watchdog {
public:
    /// Constructor.
    /// @param name             Optional name of the watchdog (appears in logs).
    explicit Watchdog(std::string_view name = "unnamed");

    /// Registers callback for given client to be called by watchdog after specified timeout expires.
    /// @param clientName       Name of the client.
    /// @param callback         Callback to be called when timeout for given client expires.
    /// @param timeout          Timeout to be set for given client.
    /// @return Flag indicating if registering was successful.
    /// @retval true            Registering was successful.
    /// @retval false           Registering was not successful.
    bool registerClient(std::string_view clientName, WatchdogCallback callback, osal::Timeout timeout);

    /// Starts the watchdog.
    /// @return Flag indicating if start was successful.
    /// @retval true            Start was successful.
    /// @retval false           Start was not successful.
    bool start();

    /// Stops the watchdog.
    /// @return Flag indicating if stop was successful.
    /// @retval true            Stop was successful.
    /// @retval false           Stop was not successful.
    bool stop();

    /// Resets timeout for the specified client.
    /// @param clientName       Client for which timeout should be restarted.
    /// @return Flag indicating if reset was successful.
    /// @retval true            Reset was successful.
    /// @retval false           Reset was not successful.
    bool reset(std::string_view clientName);

private:
    /// Internal thread function. It is responsible for calculating earliest available deadline to be used in
    /// blocking on the semaphore. Once this timeout expires, it searches for the first expired timeout and calls its
    /// handler.
    void threadFunc();

private:
    /// Represents internal meta data about the clients.
    struct ClientData {
        WatchdogCallback callback;
        osal::Timeout timeout;
    };

    static constexpr unsigned int m_cWatchdogThreadStackSize = 128 * 1024;

    std::string m_name;
    std::map<std::string, ClientData> m_clients;
    bool m_running{};
    osal::Mutex m_mutex;
    osal::Semaphore m_startSemaphore{0};
    std::unique_ptr<osal::NormalPrioThread<m_cWatchdogThreadStackSize>> m_thread;
    osal::Semaphore m_semaphore{0};
};

} // namespace utils::watchdog
