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
#include <fmt/printf.h>

REGISTER_LOGGER(Demo1Logger, "DEMO1", spdlog::level::off);
REGISTER_LOGGER(Demo2Logger, "DEMO2", spdlog::level::critical);
REGISTER_LOGGER(Demo3Logger, "DEMO3", spdlog::level::trace);
REGISTER_LOGGER(Demo4Logger, "DEMO4", spdlog::level::info);

TEST_CASE("1. Log messages are displayed correctly", "[unit][Logger]")
{
    fmt::print("\n>>>>> Using explicitly set 'OFF' level at construction time. <<<<<\n");
    Demo1Logger::trace("This trace log shouldn't be visible");
    Demo1Logger::debug("This debug log shouldn't be visible");
    Demo1Logger::info("This info log shouldn't be visible");
    Demo1Logger::warn("This warn log shouldn't be visible");
    Demo1Logger::error("This error log shouldn't be visible");
    Demo1Logger::critical("This critical log shouldn't be visible");

    fmt::print("\n>>>>> Using explicitly set 'INFO' level at runtime. <<<<<\n");
    Demo2Logger::get()->set_level(spdlog::level::info);
    Demo2Logger::trace("This trace log shouldn't be visible");
    Demo2Logger::debug("This debug log shouldn't be visible");
    Demo2Logger::info("This info log will be visible");
    Demo2Logger::warn("This warn log will be visible");
    Demo2Logger::error("This error log will be visible");
    Demo2Logger::critical("This critical log will be visible");

    fmt::print("\n>>>>> Using explicitly set 'TRACE' level at construction time. <<<<<\n");
    Demo3Logger::trace("This trace log will be visible");
    Demo3Logger::debug("This debug log will be visible");
    Demo3Logger::info("This info log will be visible");
    Demo3Logger::warn("This warn log will be visible");
    Demo3Logger::error("This error log will be visible");
    Demo3Logger::critical("This critical log will be visible");

    fmt::print("\n>>>>> Using explicitly set 'INFO' level at construction time. <<<<<\n");
    Demo4Logger::trace("This trace log shouldn't be visible");
    Demo4Logger::debug("This debug log shouldn't be visible");
    Demo4Logger::info("This info log shouldn't be visible");
    Demo4Logger::warn("This warn log shouldn't be visible");
    Demo4Logger::error("This error log shouldn't be visible");
    Demo4Logger::critical("This critical log will be visible");
}
