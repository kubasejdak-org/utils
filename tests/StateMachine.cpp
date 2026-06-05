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

#include <osal/Thread.hpp>
#include <osal/sleep.hpp>
#include <utils/sm/IState.hpp>
#include <utils/sm/StateMachine.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <utility>

struct IAppState : utils::sm::IState<IAppState> {
    IAppState(std::string name, utils::sm::StateMachine<IAppState>* stateMachine)
        : utils::sm::IState<IAppState>(std::move(name), stateMachine)
    {}

    void onEnter() override {}

    void onLeave() override {}

    virtual void func() = 0;
};

struct StateA : IAppState {
    explicit StateA(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateA", stateMachine)
    {}

    void func() override { osal::sleep(1ms); }
};

struct StateB : IAppState {
    explicit StateB(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateB", stateMachine)
    {}

    void func() override { osal::sleep(2ms); }
};

struct StateC : IAppState {
    explicit StateC(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateC", stateMachine)
    {}

    void func() override { osal::sleep(3ms); }
};

struct StateF;

struct StateD : IAppState {
    explicit StateD(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateD", stateMachine)
    {}

    void func() override
    {
        changeState<StateF>();
        osal::sleep(1ms);
    }
};

struct StateE : IAppState {
    explicit StateE(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateE", stateMachine)
    {}

    void func() override
    {
        changeState<StateD>();
        osal::sleep(2ms);
    }
};

struct StateF : IAppState {
    explicit StateF(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateF", stateMachine)
    {}

    void func() override
    {
        changeState<StateE>();
        osal::sleep(3ms);
    }
};

struct StateG : IAppState {
    explicit StateG(utils::sm::StateMachine<IAppState>* stateMachine)
        : IAppState("StateG", stateMachine)
    {}

    void func() override
    {
        changeState<StateG>();
        osal::sleep(1ms);
    }
};

struct TrackingState : IAppState {
    TrackingState(utils::sm::StateMachine<IAppState>* stateMachine, int& enterCount, int& leaveCount)
        : IAppState("TrackingState", stateMachine)
        , m_enterCount(enterCount)
        , m_leaveCount(leaveCount)
    {}

    void onEnter() override { ++m_enterCount; }

    void onLeave() override { ++m_leaveCount; }

    void func() override {}

private:
    int& m_enterCount;
    int& m_leaveCount;
};

TEST_CASE("1. Simple change of states from the outside", "[unit][StateMachine]")
{
    utils::sm::StateMachine<IAppState> stateMachine("Test");

    stateMachine.changeState<StateA>();
    CHECK(stateMachine.currentState()->name() == "StateA");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateB>();
    CHECK(stateMachine.currentState()->name() == "StateB");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateC>();
    CHECK(stateMachine.currentState()->name() == "StateC");
    stateMachine.currentState()->func();
    stateMachine.currentState()->func();

    stateMachine.changeState<StateA>();
    CHECK(stateMachine.currentState()->name() == "StateA");
    stateMachine.changeState<StateB>();
    CHECK(stateMachine.currentState()->name() == "StateB");
    stateMachine.changeState<StateC>();
    CHECK(stateMachine.currentState()->name() == "StateC");
    stateMachine.changeState<StateB>();
    CHECK(stateMachine.currentState()->name() == "StateB");
    stateMachine.changeState<StateA>();
    CHECK(stateMachine.currentState()->name() == "StateA");
    stateMachine.currentState()->func();
}

TEST_CASE("2. Changing state in a loop", "[unit][StateMachine]")
{
    utils::sm::StateMachine<IAppState> stateMachine("Test");
    constexpr int cIterations = 1000;
    std::string name;
    std::string nextName;

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

            CHECK(stateMachine.currentState()->name() == name);
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

            CHECK(stateMachine.currentState()->name() == name);
            stateMachine.currentState()->func();
            CHECK(stateMachine.currentState()->name() == nextName);
        }
    }

    SECTION("2.3. Changing state from both the sides")
    {
        for (int i = 0; i < cIterations; ++i) {
            switch (i % 6) {
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
                    break;
            }

            CHECK(stateMachine.currentState()->name() == name);
            stateMachine.currentState()->func();
            CHECK(stateMachine.currentState()->name() == nextName);
        }
    }

    SECTION("2.4. Changing state to self from outside")
    {
        for (int i = 0; i < cIterations; ++i) {
            stateMachine.changeState<StateA>();
            CHECK(stateMachine.currentState()->name() == "StateA");
            stateMachine.currentState()->func();
            CHECK(stateMachine.currentState()->name() == "StateA");
        }
    }

    SECTION("2.5. Changing state to self from inside")
    {
        for (int i = 0; i < cIterations; ++i) {
            stateMachine.changeState<StateG>();
            CHECK(stateMachine.currentState()->name() == "StateG");
            stateMachine.currentState()->func();
            CHECK(stateMachine.currentState()->name() == "StateG");
        }
    }
}

TEST_CASE("3. Changing state from multiple threads", "[unit][StateMachine]")
{
    utils::sm::StateMachine<IAppState> stateMachine("Test");
    stateMachine.changeState<StateA>();

    bool stop{};
    auto threadFunc = [&] {
        while (!stop)
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

struct ITestState : utils::sm::IState<ITestState> {
    ITestState(std::string name, utils::sm::StateMachine<ITestState>* stateMachine)
        : utils::sm::IState<ITestState>(std::move(name), stateMachine)
    {}

    virtual void func() = 0;
};

struct TestStateD;

struct TestStateA : ITestState {
    explicit TestStateA(utils::sm::StateMachine<ITestState>* stateMachine)
        : ITestState("TestStateA", stateMachine)
    {}

    void func() override { changeState<TestStateD>(); }
};

struct TestStateB : ITestState {
    explicit TestStateB(utils::sm::StateMachine<ITestState>* stateMachine)
        : ITestState("TestStateB", stateMachine)
    {}

    void onEnter() override { changeState<TestStateA>(); }

    void func() override {}
};

struct TestStateC : ITestState {
    explicit TestStateC(utils::sm::StateMachine<ITestState>* stateMachine)
        : ITestState("TestStateC", stateMachine)
    {}

    void onEnter() override { changeState<TestStateB>(); }

    void func() override {}
};

struct TestStateD : ITestState {
    explicit TestStateD(utils::sm::StateMachine<ITestState>* stateMachine)
        : ITestState("TestStateD", stateMachine)
    {}

    void onEnter() override { changeState<TestStateC>(); }

    void func() override {}
};

TEST_CASE("4. Recursive calls to changeState()", "[unit][StateMachine]")
{
    utils::sm::StateMachine<ITestState> stateMachine("Test");

    stateMachine.changeState<TestStateA>();
    CHECK(stateMachine.currentState()->name() == "TestStateA");

    SECTION("4.1. Calls triggered from outside")
    {
        stateMachine.changeState<TestStateD>();
    }

    SECTION("4.2. Calls triggered from inside")
    {
        stateMachine.currentState()->func();
    }

    CHECK(stateMachine.currentState()->name() == "TestStateA");
}

TEST_CASE("5. onEnter() and onLeave() are called correctly during state transitions", "[unit][StateMachine]")
{
    utils::sm::StateMachine<IAppState> stateMachine("Test");

    int enterCount{};
    int leaveCount{};

    stateMachine.changeState<TrackingState>(enterCount, leaveCount);
    CHECK(enterCount == 1);
    CHECK(leaveCount == 0);

    stateMachine.changeState<StateA>();
    CHECK(enterCount == 1);
    CHECK(leaveCount == 1);

    stateMachine.changeState<TrackingState>(enterCount, leaveCount);
    CHECK(enterCount == 2);
    CHECK(leaveCount == 1);

    stateMachine.changeState<TrackingState>(enterCount, leaveCount);
    CHECK(enterCount == 3);
    CHECK(leaveCount == 2);
}
