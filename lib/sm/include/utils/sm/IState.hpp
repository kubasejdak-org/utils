/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2021 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <utility>

namespace utils::sm {

template <typename UserState>
class StateMachine;

/// Base class for the user state API class, that will be used with the StateMachine.
/// @tparam UserState           Type representing base class (API) for all user state classes.
template <typename UserState>
class IState {
public:
    /// Constructor.
    /// @param name             Human-readable name of the state.
    /// @param stateMachine     Pointer to the state machine, which will manage this state.
    IState(std::string name, StateMachine<UserState>* stateMachine)
        : m_name(std::move(name))
        , m_stateMachine(stateMachine)
    {}

    /// Default copy constructor.
    IState(const IState<UserState>&) = default;

    /// Default move constructor.
    IState(IState<UserState>&&) noexcept = default;

    /// Default virtual destructor.
    virtual ~IState() = default;

    /// Default copy assignment operator.
    /// @return Reference to self.
    IState<UserState>& operator=(const IState<UserState>&) = default;

    /// Default move assignment operator.
    /// @return Reference to self.
    IState<UserState>& operator=(IState<UserState>&&) noexcept = default;

    /// Returns user-provided name of the class.
    /// @return User-provided name of the class.
    [[nodiscard]] std::string name() const { return m_name; }

    /// Method which will be called directly after setting this state as current state in state machine.
    /// @note User states can override this method for custom behavior.
    virtual void onEnter() {}

    /// Method which will be called directly before switching this state with different one as current state
    /// in state machine.
    /// @note User states can override this method for custom behavior.
    virtual void onLeave() {}

protected:
    /// Triggers change of current state in the state machine.
    /// @tparam NewState        Type of the state to be set.
    /// @tparam Args            Pack of argument types that will passed to the constructor of the new state.
    /// @param args             Pack of arguments that will passed to the constructor of the new state.
    /// @note Change of state will happen directly after function that made a call to this method.
    /// @note This operation is thread-safe.
    template <typename NewState, typename... Args>
    void changeState(Args&&... args)
    {
        m_stateMachine->template scheduleStateChange<NewState>(std::forward<Args>(args)...);
    }

private:
    std::string m_name;
    StateMachine<UserState>* m_stateMachine;
};

} // namespace utils::sm
