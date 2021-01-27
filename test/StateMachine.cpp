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

#include <osal/Thread.hpp>
#include <osal/sleep.hpp>
#include <utils/fsm/IState.hpp>
#include <utils/fsm/StateMachine.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <string_view>
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

    void func() override { osal::sleep(1ms); }
};

struct StateB : IAppState {
    explicit StateB(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateB", stateMachine)
    {}

    void func() override { osal::sleep(2ms); }
};

struct StateC : IAppState {
    explicit StateC(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateC", stateMachine)
    {}

    void func() override { osal::sleep(3ms); }
};

struct StateF;

struct StateD : IAppState {
    explicit StateD(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateD", stateMachine)
    {}

    void func() override
    {
        changeState<StateF>();
        osal::sleep(1ms);
    }
};

struct StateE : IAppState {
    explicit StateE(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateE", stateMachine)
    {}

    void func() override
    {
        changeState<StateD>();
        osal::sleep(2ms);
    }
};

struct StateF : IAppState {
    explicit StateF(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateF", stateMachine)
    {}

    void func() override
    {
        changeState<StateE>();
        osal::sleep(3ms);
    }
};

struct StateG : IAppState {
    explicit StateG(utils::fsm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateG", stateMachine)
    {}

    void func() override
    {
        changeState<StateG>();
        osal::sleep(1ms);
    }
};

TEST_CASE("1. Simple change of states from the outside", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");

    stateMachine.changeState<StateA>();
    REQUIRE(stateMachine.currentState()->name() == "StateA");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateB>();
    REQUIRE(stateMachine.currentState()->name() == "StateB");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateC>();
    REQUIRE(stateMachine.currentState()->name() == "StateC");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateA>();
    REQUIRE(stateMachine.currentState()->name() == "StateA");
    stateMachine.changeState<StateB>();
    REQUIRE(stateMachine.currentState()->name() == "StateB");
    stateMachine.changeState<StateC>();
    REQUIRE(stateMachine.currentState()->name() == "StateC");
    stateMachine.changeState<StateB>();
    REQUIRE(stateMachine.currentState()->name() == "StateB");
    stateMachine.changeState<StateA>();
    REQUIRE(stateMachine.currentState()->name() == "StateA");
    stateMachine.currentState()->func();
}

TEST_CASE("2. Changing state in a loop", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");
    constexpr int cIterations = 1'000;
    std::string_view name;
    std::string_view nextName;

    SECTION("2.1. Changing state from the outside")
    {
        for (int i = 0; i < cIterations; ++i) {
            switch (i % 3) {
                case 0:
                    stateMachine.changeState<StateA>();
                    name = "StateA";
                    break;
                case 1:
                    stateMachine.changeState<StateB>();
                    name = "StateB";
                    break;
                case 2:
                    stateMachine.changeState<StateC>();
                    name = "StateC";
                    break;
            }

            REQUIRE(stateMachine.currentState()->name() == name);
            stateMachine.currentState()->func();
        }
    }

    SECTION("2.2. Changing state from the inside")
    {
        for (int i = 0; i < cIterations; ++i) {
            switch (i % 3) {
                case 0:
                    stateMachine.changeState<StateD>();
                    name = "StateD";
                    nextName = "StateF";
                    break;
                case 1:
                    stateMachine.changeState<StateE>();
                    name = "StateE";
                    nextName = "StateD";
                    break;
                case 2:
                    stateMachine.changeState<StateF>();
                    name = "StateF";
                    nextName = "StateE";
                    break;
            }

            REQUIRE(stateMachine.currentState()->name() == name);
            stateMachine.currentState()->func();
            REQUIRE(stateMachine.currentState()->name() == nextName);
        }
    }

    SECTION("2.3. Changing state from both the sides")
    {
        for (int i = 0; i < cIterations; ++i) {
            switch (i % 6) { // NOLINT
                case 0:
                    stateMachine.changeState<StateA>();
                    name = "StateA";
                    nextName = name;
                    break;
                case 1:
                    stateMachine.changeState<StateD>();
                    name = "StateD";
                    nextName = "StateF";
                    break;
                case 2:
                    stateMachine.changeState<StateE>();
                    name = "StateE";
                    nextName = "StateD";
                    break;
                case 3:
                    stateMachine.changeState<StateB>();
                    name = "StateB";
                    nextName = name;
                    break;
                case 4:
                    stateMachine.changeState<StateC>();
                    name = "StateC";
                    nextName = name;
                    break;
                case 5:
                    stateMachine.changeState<StateF>();
                    name = "StateF";
                    nextName = "StateE";
                    break; // NOLINT
            }

            REQUIRE(stateMachine.currentState()->name() == name);
            stateMachine.currentState()->func();
            REQUIRE(stateMachine.currentState()->name() == nextName);
        }
    }

    SECTION("2.4. Changing state to self from outside")
    {
        for (int i = 0; i < cIterations; ++i) {
            stateMachine.changeState<StateA>();
            REQUIRE(stateMachine.currentState()->name() == "StateA");
            stateMachine.currentState()->func();
            REQUIRE(stateMachine.currentState()->name() == "StateA");
        }
    }

    SECTION("2.5. Changing state to self from inside")
    {
        for (int i = 0; i < cIterations; ++i) {
            stateMachine.changeState<StateG>();
            REQUIRE(stateMachine.currentState()->name() == "StateG");
            stateMachine.currentState()->func();
            REQUIRE(stateMachine.currentState()->name() == "StateG");
        }
    }
}

TEST_CASE("3. Changing state from multiple threads", "[unit][StateMachine]")
{
    utils::fsm::StateMachine<IAppState> stateMachine("Test");
    stateMachine.changeState<StateA>();

    bool stop{};
    auto threadFunc = [&] {
        while (!stop) // NOLINT
            stateMachine.currentState()->func();
    };

    constexpr int cThreadsCount = 20;
    std::array<osal::Thread<>, cThreadsCount> normalThreads = {
        osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc},
        osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc},
        osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc},
        osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc},
        osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}, osal::Thread<>{threadFunc}};

    osal::Thread<> changeThread1([&] {
        stateMachine.changeState<StateA>();
        osal::sleep(1ms);
    });
    osal::Thread<> changeThread2([&] {
        stateMachine.changeState<StateB>();
        osal::sleep(2ms);
    });
    osal::Thread<> changeThread3([&] {
        stateMachine.changeState<StateC>();
        osal::sleep(3ms);
    });
    osal::Thread<> changeThread4([&] {
        stateMachine.changeState<StateD>();
        osal::sleep(4ms);
    });
    osal::Thread<> changeThread5([&] {
        stateMachine.changeState<StateE>();
        osal::sleep(3ms);
    });
    osal::Thread<> changeThread6([&] {
        stateMachine.changeState<StateF>();
        osal::sleep(2ms);
    });
    osal::Thread<> changeThread7([&] {
        stateMachine.changeState<StateG>();
        osal::sleep(1ms);
    });

    osal::sleep(30s);
    stop = true;
}
