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
#include <osal/ScopedLock.hpp>
#include <utils/fsm/IState.hpp>
#include <utils/fsm/logger.hpp>

#include <fmt/printf.h>

#include <functional>
#include <type_traits>

namespace utils::fsm {

template <typename UserState>
class StateMachine {
    static_assert(std::is_base_of_v<IState<UserState>, UserState>);

public:
    explicit StateMachine(std::string name = "<unnamed>")
        : m_name(std::move(name))
    {}

    template <typename NewState, typename... Args>
    void changeState(Args&&... args, bool callFromState = false)
    {
        static_assert(std::is_base_of_v<UserState, NewState>);

        osal::ScopedLock lock(m_mutex);
        m_newState = std::make_shared<NewState>(this, std::forward<Args>(args)...);

        ++m_changeStateCounter;

        if (!callFromState)
            changeStatePriv();
    }

    [[nodiscard]] auto currentState()
    {
        using UserStateRef = std::reference_wrapper<std::shared_ptr<UserState>>;
        return ExecAround<UserStateRef>([this] { preStateCall(); },
                                        [this] { postStateCall(); },
                                        std::ref(m_currentState));
    }

private:
    void preStateCall()
    {
        m_mutex.lock();
        FsmLogger::debug("<{}:{}> preStateCall", m_name, m_currentState->name());
    };

    void postStateCall()
    {
        FsmLogger::debug("<{}:{}> postStateCall", m_name, m_currentState->name());

        if (m_newState)
            changeStatePriv();

        m_mutex.unlock();
    };

    void changeStatePriv()
    {
        if (m_currentState) {
            FsmLogger::info("<{}:{}> Leaving state", m_name, m_currentState->name());
            m_currentState->onLeave();
        }

        m_currentState = std::move(m_newState);

        FsmLogger::info("<{}:{}> Entering state", m_name, m_currentState->name());
        m_currentState->onEnter();

        if (m_changeStateCounter > 1)
            FsmLogger::error("<{}:{}> Recursive changeState() called", m_name, m_currentState->name());

        m_changeStateCounter = 0;
    }

private:
    std::string m_name;
    osal::Mutex m_mutex{OsalMutexType::eRecursive};
    std::size_t m_changeStateCounter{};
    std::shared_ptr<UserState> m_currentState{};
    std::shared_ptr<UserState> m_newState{};
};

} // namespace utils::fsm
