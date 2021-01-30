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
#include <utils/functional/ExecuteAround.hpp>

#include <cassert>
#include <functional>
#include <type_traits>

namespace utils::fsm {

/// Class representing a thread-safe state machine.
/// This state machine assumes, that user states have one common base class which acts as an interface (API) for all its
/// subclasses. State machine is implemented in a way, that it holds internally an object of current state and provides
/// an access to it with a currentState() method. All mechanisms are thread-safe, meaning that both calls to the state's
/// methods are synchronized between each other and also change of states can be done at any time by any number of
/// concurrent threads.
/// There can be more than one state machine in a system which work independently from each other. It is up to the user
/// to provide synchronization (if needed) between states used in more than one state machine.
/// @tparam UserState           Type representing base class (API) for all user state classes.
/// @note UserState has to be derived from IState<UserState>.
/// @note During change of state, the old state is being destroyed and the new one is being constructed with a given
///       arguments.
template <typename UserState>
class StateMachine {
    static_assert(std::is_base_of_v<IState<UserState>, UserState>);

public:
    /// Constructor.
    /// @param name             Created an uninitialized state machine with a given name.
    explicit StateMachine(std::string name = "<unnamed>")
        : m_name(std::move(name))
    {}

    /// Triggers change of current state in the state machine.
    /// @tparam NewState        Type of the state to be set.
    /// @tparam Args            Pack of argument types that will passed to the constructor of the new state.
    /// @param args             Pack of arguments that will passed to the constructor of the new state.
    /// @note This method is thread-safe.
    /// @note This method is able to detect recursive usage (it is reported with a proper log in changeStatePriv()
    ///       method). It is not explicitly disallowed to do that, however bare in mind that this can lead to hard
    ///       to track bugs.
    template <typename NewState, typename... Args>
    void changeState(Args&&... args)
    {
        osal::ScopedLock lock(m_mutex);

        scheduleStateChange<NewState>(std::forward<Args>(args)...);
        executeStateChange();
    }

    /// Returns object of the currently set state.
    /// @return Object of the currently set state.
    /// @note The returned object is wrapped with an ExecAround class to allow working of some of the state machine's
    ///       mechanisms.
    /// @note This method is thread-safe.
    [[nodiscard]] auto currentState()
    {
        using UserStateRef = std::reference_wrapper<std::shared_ptr<UserState>>;
        return functional::ExecAround<UserStateRef>([this] { preStateCall(); },
                                                    [this] { postStateCall(); },
                                                    std::ref(m_currentState));
    }

private:
    /// Method which will be called directly before every call to the state's methods. It locks the internal state of
    /// the state machine.
    void preStateCall()
    {
        m_mutex.lock();
        FsmLogger::debug("<{}:{}> preStateCall", m_name, m_currentState->name());
    };

    /// Method which will be called directly after every call to the state's methods. If a state change was triggered,
    /// then it will execute it (still with the internal state being locked). Before returning to the caller the
    /// internal lock is released.
    void postStateCall()
    {
        FsmLogger::debug("<{}:{}> postStateCall", m_name, m_currentState->name());

        if (m_newState)
            executeStateChange();

        m_mutex.unlock();
    };

    /// Prepares new state, that will be used during the execute state change step.
    /// @tparam NewState        Type of the state to be set.
    /// @tparam Args            Pack of argument types that will passed to the constructor of the new state.
    /// @param args             Pack of arguments that will passed to the constructor of the new state.
    /// @note This method is thread-safe.
    /// @note This method can be used only from the state function.
    template <typename NewState, typename... Args>
    void scheduleStateChange(Args&&... args)
    {
        static_assert(std::is_base_of_v<UserState, NewState>);

        assert(!m_newState);
        m_newState = std::make_shared<NewState>(this, std::forward<Args>(args)...);

        ++m_changeStateCounter;
    }

    /// Executes change of state. It is assumed, that new state is already constructed in a helper class field.
    /// This method automatically calls onLeave() method of the old state and onEnter() method of the new state.
    /// @note This method is checking the value of the internal counter of recursive usage of changeState() method. In
    ///       case of recursive usage, it will report it with a proper log message.
    void executeStateChange()
    {
        if (m_currentState) {
            FsmLogger::info("<{}:{}> Leaving state", m_name, m_currentState->name());
            m_currentState->onLeave();
        }

        m_currentState = std::move(m_newState);

        FsmLogger::info("<{}:{}> Entering state", m_name, m_currentState->name());
        m_currentState->onEnter();

        if (m_newState) {
            // This can happen, if onEnter() method in the state class calls changeState() recursively.
            executeStateChange();
        }

        if (m_changeStateCounter > 1) {
            FsmLogger::warn("<{}:{}> Recursive calls to changeState() detected: called {} times",
                            m_name,
                            m_currentState->name(),
                            m_changeStateCounter);
        }

        m_changeStateCounter = 0;
    }

private:
    friend class IState<UserState>;

    std::string m_name;
    osal::Mutex m_mutex{OsalMutexType::eRecursive};
    std::size_t m_changeStateCounter{};
    std::shared_ptr<UserState> m_currentState{};
    std::shared_ptr<UserState> m_newState{};
};

} // namespace utils::fsm
