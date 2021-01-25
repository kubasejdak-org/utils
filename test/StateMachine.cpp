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

#include <utils/fsm/ExecuteAround.hpp>
#include <utils/fsm/IState.hpp>
#include <utils/fsm/StateMachine.hpp>

#include <catch2/catch.hpp>

#include <utility>

struct IAppState : utils::fsm::IState<IAppState> {
    IAppState(std::string name, utils::fsm::StateMachine<IAppState>* stateMachine)
        : utils::fsm::IState<IAppState>(std::move(name), stateMachine)
    {}

    void onEnter() override {}
    void onLeave() override {}

    virtual void func() = 0;
};

struct StateA : IAppState {
    explicit StateA(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateA", stateMachine)
    {}

    void func() override {}
};

struct StateB : IAppState {
    explicit StateB(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateB", stateMachine)
    {}

    void func() override {}
};

struct StateD;

struct StateC : IAppState {
    explicit StateC(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateC", stateMachine)
    {}

    void func() override { changeState<StateD>(); }
};

struct StateD : IAppState {
    explicit StateD(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateD", stateMachine)
    {}

    void func() override {}
};

TEST_CASE("1. Simple change of states from the outside", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");

    stateMachine.changeState<StateA>();
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateB>();
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateD>();
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateA>();
    stateMachine.changeState<StateB>();
    stateMachine.changeState<StateD>();
    stateMachine.changeState<StateB>();
    stateMachine.changeState<StateA>();
    stateMachine.currentState()->func();
}

TEST_CASE("2. Changing state from the outside in a loop", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");

    constexpr int cIterations = 100'000;
    for (int i = 0; i < cIterations; ++i) {
        switch (i % 3) {
            case 0: stateMachine.changeState<StateA>(); break;
            case 1: stateMachine.changeState<StateB>(); break;
            case 2: stateMachine.changeState<StateD>(); break;
        }

        stateMachine.currentState()->func();
    }
}

TEST_CASE("3. Changing state from the inside and outside in a loop", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");

    constexpr int cIterations = 100'000;
    for (int i = 0; i < cIterations; ++i) {
        switch (i % 4) {
            case 0: stateMachine.changeState<StateA>(); break;
            case 1: stateMachine.changeState<StateB>(); break;
            case 2: stateMachine.changeState<StateC>(); break;
            case 3: stateMachine.changeState<StateD>(); break;
        }

        stateMachine.currentState()->func();
    }
}
