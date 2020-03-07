/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/GlobalRegistry.hpp>

#include <catch2/catch.hpp>

#include <memory>
#include <type_traits>

TEST_CASE("All instances are correctly stored in GlobalRegistry", "[unit][GlobalRegistry]")
{
    struct Test : utils::Registrable<int> {
        explicit Test(int a)
            : utils::Registrable<int>(a)
        {}
    };

    std::size_t instancesCount{};

    SECTION("1 instance")
    {
        utils::GlobalRegistry<Test>::init(Test(0));
        instancesCount = 1;
    }

    SECTION("4 instances")
    {
        utils::GlobalRegistry<Test>::init(Test(0), Test(1), Test(2), Test(3));
        instancesCount = 4;
    }

    auto size = utils::GlobalRegistry<Test>::size();
    REQUIRE(size == instancesCount);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = utils::GlobalRegistry<Test>::get(i);
        REQUIRE(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance->instanceId() == static_cast<Test::InstanceIdType>(i));
    }

    utils::GlobalRegistry<Test>::clear();
}

TEST_CASE("Copy only types can be stored in GlobalRegistry", "[unit][GlobalRegistry]")
{
    struct Test : utils::Registrable<int> {
        explicit Test(int a)
            : utils::Registrable<int>(a)
        {}

        Test(const Test& other)
            : utils::Registrable<int>(other)
        {
            copied = true;
        }
        Test(Test&&) = delete;

        bool copied{false};
    };

    std::size_t instancesCount{};

    SECTION("1 instance")
    {
        Test instance0(0);
        utils::GlobalRegistry<Test>::init(instance0);
        instancesCount = 1;
    }

    SECTION("4 instances")
    {
        Test instance0(0);
        Test instance1(1);
        Test instance2(2);
        Test instance3(3);
        utils::GlobalRegistry<Test>::init(instance0, instance1, instance2, instance3);
        instancesCount = 4;
    }

    auto size = utils::GlobalRegistry<Test>::size();
    REQUIRE(size == instancesCount);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = utils::GlobalRegistry<Test>::get(i);
        REQUIRE(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance->instanceId() == static_cast<Test::InstanceIdType>(i));
        REQUIRE(instance->copied);
    }

    utils::GlobalRegistry<Test>::clear();
}

TEST_CASE("Move only types can be stored in GlobalRegistry", "[unit][GlobalRegistry]")
{
    struct Test : utils::Registrable<int> {
        explicit Test(int a)
            : utils::Registrable<int>(a)
        {}

        Test(const Test&) = delete;
        Test(Test&& other) noexcept
            : utils::Registrable<int>(std::move(other))
        {
            moved = true;
        }

        bool moved{false};
    };

    std::size_t instancesCount{};

    SECTION("1 instance")
    {
        utils::GlobalRegistry<Test>::init(Test(0));
        instancesCount = 1;
    }

    SECTION("4 instances")
    {
        utils::GlobalRegistry<Test>::init(Test(0), Test(1), Test(2), Test(3));
        instancesCount = 4;
    }

    auto size = utils::GlobalRegistry<Test>::size();
    REQUIRE(size == instancesCount);
    for (std::size_t i = 0; i < size; ++i) {
        auto instance = utils::GlobalRegistry<Test>::get(i);
        REQUIRE(std::is_same_v<decltype(instance), std::shared_ptr<Test>>);
        REQUIRE(instance->instanceId() == static_cast<Test::InstanceIdType>(i));
        REQUIRE(instance->moved);
    }

    utils::GlobalRegistry<Test>::clear();
}
