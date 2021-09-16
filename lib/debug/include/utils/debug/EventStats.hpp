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
#include <osal/timestamp.hpp>

#include <chrono>
#include <deque>
#include <optional>
#include <string>

namespace utils::debug {

using EventList = std::deque<osal::Timestamp>;
using Period = std::chrono::milliseconds;
using PeriodList = std::deque<Period>;

class EventStats {
public:
    explicit EventStats(std::optional<std::size_t> capacity = 1000,
                        bool start = false,
                        std::string_view name = "unnamed");

    void startTimer();
    void stopTimer();

    [[nodiscard]] auto elapsed() const;
    [[nodiscard]] auto elapsedSec() const;
    [[nodiscard]] auto elapsedMs() const;
    [[nodiscard]] auto elapsedNs() const;

    void event();
    std::size_t eventsCount();
    Period eventsPeriodAvg();
    Period eventsPeriodMin();
    Period eventsPeriodMax();

private:
    PeriodList getPeriods();

private:
    std::optional<std::size_t> m_capacity;
    std::string m_name;
    bool m_started{};
    osal::Timestamp m_start;
    osal::Timestamp m_end;
    osal::Mutex m_mutex;
    EventList m_events;
};

} // namespace utils::debug
