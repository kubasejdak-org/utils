/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2021-2022, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/functional/ExecAround.hpp>

#include <catch2/catch.hpp>

#include <functional>
#include <memory>

struct TestType {
    TestType() = default;

    TestType(const TestType& /*unused*/)
        : copyConstructed(true)
    {}

    TestType(TestType&& /*unused*/) noexcept
        : moveConstructed(true)
    {}

    ~TestType() = default;

    TestType& operator=(const TestType& /*unused*/) // NOLINT
    {
        copyAssigned = true;
        return *this;
    }

    TestType& operator=(TestType&& /*unused*/) noexcept
    {
        moveAssigned = true;
        return *this;
    }

    bool copyConstructed{};
    bool moveConstructed{};
    bool copyAssigned{};
    bool moveAssigned{};
    int i{};
    double d{};

    void func1() { ++i; }

    void func2() { ++d; }
};

TEST_CASE("1. Simple wrapper around custom type", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);
    REQUIRE(wrapper->i == 0);
    REQUIRE(wrapper->d == 0.0);

    wrapper->func1();
    REQUIRE(wrapper->i == 1);
    REQUIRE(wrapper->d == 0.0);
    REQUIRE(preCall);
    REQUIRE(postCall);

    preCall = false;
    postCall = false;

    wrapper->func2();
    REQUIRE(wrapper->i == 1);
    REQUIRE(wrapper->d == 1.0);
    REQUIRE(preCall);
    REQUIRE(postCall);
}

TEST_CASE("2. Moving ExecAround around", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);

    SECTION("2.1. Copy construction")
    {
        auto copiedWrapper = wrapper;
        REQUIRE(copiedWrapper->copyConstructed);

        copiedWrapper->func1();
        REQUIRE(copiedWrapper->i == 1);
    }

    SECTION("2.2. Move construction")
    {
        auto movedWrapper(std::move(wrapper));
        REQUIRE(movedWrapper->moveConstructed);

        movedWrapper->func1();
        REQUIRE(movedWrapper->i == 1);
    }

    SECTION("2.3. Copy assignment")
    {
        utils::functional::ExecAround<TestType> copyAssignedWrapper;
        copyAssignedWrapper = wrapper; // NOLINT
        REQUIRE(copyAssignedWrapper->copyAssigned);

        copyAssignedWrapper->func1();
        REQUIRE(copyAssignedWrapper->i == 1);
    }

    SECTION("2.4. Move assignment")
    {
        utils::functional::ExecAround<TestType> moveAssignedWrapper;
        moveAssignedWrapper = std::move(wrapper);
        REQUIRE(moveAssignedWrapper->moveAssigned);

        moveAssignedWrapper->func1();
        REQUIRE(moveAssignedWrapper->i == 1);
    }

    REQUIRE(wrapper->i == 0); // NOLINT
    REQUIRE(preCall);
    REQUIRE(postCall);
}

TEST_CASE("3. Passing wrapped object in different ways", "[unit][ExecAround]")
{
    bool preCall{};
    bool postCall{};
    auto preAction = [&] { preCall = true; };
    auto postAction = [&] { postCall = true; };

    SECTION("3.1. Copy from object")
    {
        TestType test;
        utils::functional::ExecAround<TestType> wrapper(test, preAction, postAction);
        REQUIRE(wrapper->copyConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test.i == 0);
    }

    SECTION("3.2. Copy from temporary")
    {
        utils::functional::ExecAround<TestType> wrapper(TestType{}, preAction, postAction);
        REQUIRE(wrapper->moveConstructed);

        wrapper->func1();
    }

    SECTION("3.3. Move")
    {
        TestType test;
        utils::functional::ExecAround<TestType> wrapper(std::move(test), preAction, postAction);
        REQUIRE(wrapper->moveConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test.i == 0); // NOLINT
    }

    SECTION("3.4. Reference wrapper")
    {
        using TestTypeRef = std::reference_wrapper<TestType>;

        TestType test;
        utils::functional::ExecAround<TestTypeRef> wrapper(std::ref(test), preAction, postAction);
        REQUIRE(!wrapper->copyConstructed);
        REQUIRE(!wrapper->moveConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test.i == 1);
    }

    SECTION("3.5. Pointer")
    {
        auto test = std::make_unique<TestType>();
        utils::functional::ExecAround<TestType*> wrapper(test.get(), preAction, postAction);
        REQUIRE(!wrapper->copyConstructed);
        REQUIRE(!wrapper->moveConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test->i == 1);
    }

    SECTION("3.6. Shared pointer")
    {
        auto test = std::make_shared<TestType>();
        utils::functional::ExecAround<std::shared_ptr<TestType>> wrapper(test, preAction, postAction);
        REQUIRE(!wrapper->copyConstructed);
        REQUIRE(!wrapper->moveConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test->i == 1);
    }

    SECTION("3.7. Reference to shared pointer")
    {
        using TestTypeSharedPtrRef = std::reference_wrapper<std::shared_ptr<TestType>>;

        auto test = std::make_shared<TestType>();
        utils::functional::ExecAround<TestTypeSharedPtrRef> wrapper(std::ref(test), preAction, postAction);
        REQUIRE(!wrapper->copyConstructed);
        REQUIRE(!wrapper->moveConstructed);

        wrapper->func1();
        REQUIRE(wrapper->i == 1);
        REQUIRE(test->i == 1);
    }

    REQUIRE(preCall);
    REQUIRE(postCall);
}
