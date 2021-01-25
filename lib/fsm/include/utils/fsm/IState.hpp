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

#include <string>
#include <utility>

namespace utils::fsm {

template <typename UserState>
class StateMachine;

template <typename UserState>
class IState {
public:
    IState(std::string name, StateMachine<UserState>* stateMachine)
        : m_name(std::move(name))
        , m_stateMachine(stateMachine)
    {}

    IState(const IState<UserState>&) = default;
    IState(IState<UserState>&&) noexcept = default;
    virtual ~IState() = default;

    IState<UserState>& operator=(const IState<UserState>&) = default;
    IState<UserState>& operator=(IState<UserState>&&) noexcept = default;

    [[nodiscard]] std::string name() const { return m_name; }

protected:
    virtual void onEnter() {}
    virtual void onLeave() {}

    template <typename NewState, typename... Args>
    void changeState(Args&&... args)
    {
        m_stateMachine->template changeState<NewState>(true, std::forward<Args>(args)...);
    }

private:
    std::string m_name;
    StateMachine<UserState>* m_stateMachine;
};

} // namespace utils::fsm
