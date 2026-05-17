/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2020 Kuba Sejdak (kuba.sejdak@gmail.com)
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

#include <utils/logger/Logger.hpp>

#include <catch2/catch_test_macros.hpp>
#include <spdlog/spdlog.h>

REGISTER_LOGGER(Demo1Logger, "DEMO1", spdlog::level::off);
REGISTER_LOGGER(Demo2Logger, "DEMO2", spdlog::level::critical);
REGISTER_LOGGER(Demo3Logger, "DEMO3", spdlog::level::trace);
REGISTER_LOGGER(Demo4Logger, "DEMO4", spdlog::level::info);

TEST_CASE("1. OFF level logger produces no output and has no underlying instance", "[unit][Logger]")
{
    CHECK(Demo1Logger::get() == nullptr);

    Demo1Logger::trace("This trace log shouldn't be visible");
    Demo1Logger::debug("This debug log shouldn't be visible");
    Demo1Logger::info("This info log shouldn't be visible");
    Demo1Logger::warn("This warn log shouldn't be visible");
    Demo1Logger::error("This error log shouldn't be visible");
    Demo1Logger::critical("This critical log shouldn't be visible");
}

TEST_CASE("2. Logger instances have correct initial state", "[unit][Logger]")
{
    SECTION("2.1. CRITICAL level")
    {
        REQUIRE(Demo2Logger::get() != nullptr);
        CHECK(Demo2Logger::get()->name() == "DEMO2");
        CHECK(Demo2Logger::get()->level() == spdlog::level::critical);

        Demo2Logger::trace("This trace log shouldn't be visible");
        Demo2Logger::debug("This debug log shouldn't be visible");
        Demo2Logger::info("This info log shouldn't be visible");
        Demo2Logger::warn("This warn log shouldn't be visible");
        Demo2Logger::error("This error log shouldn't be visible");
        Demo2Logger::critical("This critical log will be visible");
    }

    SECTION("2.2. TRACE level")
    {
        REQUIRE(Demo3Logger::get() != nullptr);
        CHECK(Demo3Logger::get()->name() == "DEMO3");
        CHECK(Demo3Logger::get()->level() == spdlog::level::trace);

        Demo3Logger::trace("This trace log will be visible");
        Demo3Logger::debug("This debug log will be visible");
        Demo3Logger::info("This info log will be visible");
        Demo3Logger::warn("This warn log will be visible");
        Demo3Logger::error("This error log will be visible");
        Demo3Logger::critical("This critical log will be visible");
    }

    SECTION("2.3. INFO level")
    {
        REQUIRE(Demo4Logger::get() != nullptr);
        CHECK(Demo4Logger::get()->name() == "DEMO4");
        CHECK(Demo4Logger::get()->level() == spdlog::level::info);

        Demo4Logger::trace("This trace log shouldn't be visible");
        Demo4Logger::debug("This debug log shouldn't be visible");
        Demo4Logger::info("This info log will be visible");
        Demo4Logger::warn("This warn log will be visible");
        Demo4Logger::error("This error log will be visible");
        Demo4Logger::critical("This critical log will be visible");
    }
}

TEST_CASE("3. Log level can be changed at runtime", "[unit][Logger]")
{
    REQUIRE(Demo2Logger::get() != nullptr);

    auto originalLevel = Demo2Logger::get()->level();
    Demo2Logger::get()->set_level(spdlog::level::info);
    CHECK(Demo2Logger::get()->level() == spdlog::level::info);

    Demo2Logger::trace("This trace log shouldn't be visible");
    Demo2Logger::debug("This debug log shouldn't be visible");
    Demo2Logger::info("This info log will be visible");
    Demo2Logger::warn("This warn log will be visible");
    Demo2Logger::error("This error log will be visible");
    Demo2Logger::critical("This critical log will be visible");

    Demo2Logger::get()->set_level(originalLevel);
}
