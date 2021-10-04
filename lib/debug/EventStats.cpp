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

#include "utils/debug/EventStats.hpp"

#include "utils/debug/logger.hpp"

#include <osal/ScopedLock.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>

namespace utils::debug {

EventStats::EventStats(std::optional<std::size_t> capacity, bool start, std::string_view name)
    : m_capacity(capacity)
    , m_name(name)
{
    if (start)
        startTimer();
}

void EventStats::startTimer()
{
    EventStatsLogger::info("<{}> Timer started\n", m_name);
    m_timerStart = osal::timestamp();
    m_timerEnd = {};
    m_started = true;
}

void EventStats::stopTimer()
{
    assert(m_started);
    m_timerEnd = osal::timestamp();
    m_started = false;
    EventStatsLogger::info("<{}> Timer stopped\n", m_name);
}

auto EventStats::elapsed() const
{
    return m_timerEnd - m_timerStart;
}

void EventStats::event()
{
    osal::ScopedLock lock(m_mutex);
    EventStatsLogger::debug("<{}> Event triggered\n", m_name);

    if (m_capacity && m_events.size() == *m_capacity)
        m_events.pop_front();

    m_events.emplace_back(osal::timestamp());
    ++m_count;

    if (m_events.size() > 1) {
        auto penultimateTimestamp = m_events[m_events.size() - 2];
        auto lastTimestamp = m_events[m_events.size() - 1];
        auto lastPeriod = std::chrono::duration_cast<Period>(lastTimestamp - penultimateTimestamp);
        m_periodOverallMin = std::min(m_periodOverallMin, lastPeriod);
        m_periodOverallMax = std::max(m_periodOverallMax, lastPeriod);
    }
}

std::size_t EventStats::eventsCount()
{
    osal::ScopedLock lock(m_mutex);
    return m_events.size();
}

Period EventStats::eventsPeriodAvg()
{
    osal::ScopedLock lock(m_mutex);
    if (m_events.empty())
        return {};

    auto periods = getPeriods();
    return std::accumulate(periods.begin(), periods.end(), 0ms) / periods.size();
}

Period EventStats::eventsPeriodMin()
{
    osal::ScopedLock lock(m_mutex);
    if (m_events.size() < 2)
        return {};

    auto periods = getPeriods();
    auto it = std::min_element(periods.begin(), periods.end());
    return *it;
}

Period EventStats::eventsPeriodMax()
{
    osal::ScopedLock lock(m_mutex);
    if (m_events.size() < 2)
        return {};

    auto periods = getPeriods();
    auto it = std::max_element(periods.begin(), periods.end());
    return *it;
}

Period EventStats::eventsPeriodOverallMin()
{
    osal::ScopedLock lock(m_mutex);
    return m_periodOverallMin;
}

Period EventStats::eventsPeriodOverallMax()
{
    osal::ScopedLock lock(m_mutex);
    return m_periodOverallMax;
}

void EventStats::clear()
{
    osal::ScopedLock lock(m_mutex);
    m_events.clear();
    m_count = 0;
    m_periodOverallMin = {};
    m_periodOverallMax = {};
}

TimestampQueue EventStats::getEvents()
{
    osal::ScopedLock lock(m_mutex);
    return m_events;
}

PeriodQueue EventStats::getPeriods()
{
    osal::ScopedLock lock(m_mutex);
    TimestampQueue periodsTmp;
    std::adjacent_difference(m_events.begin(), m_events.end(), std::back_inserter(periodsTmp));
    periodsTmp.pop_front();

    PeriodQueue periods;
    std::transform(periodsTmp.begin(), periodsTmp.end(), std::back_inserter(periods), [](const auto& period) {
        return std::chrono::duration_cast<Period>(period);
    });
    return periods;
}

} // namespace utils::debug
