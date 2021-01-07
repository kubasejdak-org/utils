/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2021, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <utils/Logger.hpp>

#include <catch2/catch.hpp>
#include <fmt/printf.h>

REGISTER_LOGGER(Demo1Logger, "DEMO1", spdlog::level::off);
REGISTER_LOGGER(Demo2Logger, "DEMO2", spdlog::level::critical);
REGISTER_LOGGER(Demo3Logger, "DEMO3", spdlog::level::trace);
REGISTER_LOGGER(Demo4Logger, "DEMO4", spdlog::level::info);

TEST_CASE("Log messages are displayed correctly", "[unit][Logger]")
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
