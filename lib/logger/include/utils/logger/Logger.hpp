/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2022, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <utility>

/// Registers custom user-defined logger with the given properties.
/// @tparam LoggerType      Type used to access given logger.
/// @param loggerName       Name of the logger (displayed as [<NAME>] in log message).
/// @param logLevel         Default logging level for this logger.
/// @param loggerCreator    Callable object which will create the logger.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_LOGGER_EX(LoggerType, loggerName, logLevel, loggerCreator)                                            \
    struct LoggerType##Tag {                                                                                           \
        static constexpr auto* name = loggerName;                                                                      \
    };                                                                                                                 \
    using LoggerType = utils::logger::ModuleLogger<LoggerType##Tag, logLevel, loggerCreator> // NOLINT

#ifdef APPLICATION_LOGGER
    #if __has_include("ApplicationLogger.hpp")
        #include "ApplicationLogger.hpp"
    #endif

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define LOGGER_CREATOR app::ApplicationLoggerCreator
#else
    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define LOGGER_CREATOR utils::logger::detail::DefaultLoggerCreator
#endif

/// Registers custom user-defined logger with the given properties.
/// @tparam LoggerType      Type used to access given logger.
/// @param loggerName       Name of the logger (displayed as [<NAME>] in log message).
/// @param logLevel         Default logging level for this logger.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_LOGGER(LoggerType, loggerName, logLevel)                                                              \
    REGISTER_LOGGER_EX(LoggerType, loggerName, logLevel, LOGGER_CREATOR)

namespace utils::logger {
namespace detail {

/// This function object is used as a default creator of the module logger.
/// Logger created by this structure has the following characteristics:
/// - it logs to the stdout (the same destination as std::printf()),
/// - it is thread safe,
/// - it uses the following format: '[MODULE_NAME][E][14:42:27] Some error message.'.
struct DefaultLoggerCreator {
    /// Constructs and registers the default module logger.
    /// @param name         Name of the logger (used as a tag in the log prefix).
    /// @param level        Default logging level used by this logger.
    void operator()(const std::string& name, spdlog::level::level_enum level)
    {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::trace);
        consoleSink->set_pattern("[%n][%^%L%$][%T.%e] %v");

        auto logger = std::make_shared<spdlog::logger>(name, consoleSink);
        logger->set_level(level);
        spdlog::register_logger(logger);
    }
};

} // namespace detail

/// Represents the container for the callable, that is used to create the module logger.
/// @param name             Name of the logger (used as a tag in the log prefix).
/// @param level            Default logging level used by this logger.
using LoggerCreator = std::function<void(const std::string& name, spdlog::level::level_enum)>;

/// Represents the logger, that has the module name as part of its type (via Tag).
/// @tparam Tag             Tag type, that will be used to obtain the module name (requires const char* name member).
/// @tparam level           Default logging level used by this logger.
/// @tparam LoggerCreator   Default creator used to create and register the logger.
template <typename Tag, spdlog::level::level_enum cLevel, typename LoggerCreator = detail::DefaultLoggerCreator>
class ModuleLoggerImpl {
public:
    /// Outputs the TRACE logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void trace(std::string_view fmt, Args&&... args)
    {
        get()->trace(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Outputs the DEBUG logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void debug(std::string_view fmt, Args&&... args)
    {
        get()->debug(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Outputs the INFO logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void info(std::string_view fmt, Args&&... args)
    {
        get()->info(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Outputs the WARN logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void warn(std::string_view fmt, Args&&... args)
    {
        get()->warn(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Outputs the ERROR logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void error(std::string_view fmt, Args&&... args)
    {
        get()->error(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Outputs the CRITICAL logs.
    /// @tparam Args        Logger arguments' types.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void critical(std::string_view fmt, Args&&... args)
    {
        get()->critical(fmt::runtime(fmt), std::forward<Args>(args)...);
    }

    /// Returns the underlying spdlog logger.
    /// @return Underlying spdlog logger.
    static std::shared_ptr<spdlog::logger> get() { return instance().m_logger; }

private:
    /// Default constructor.
    ModuleLoggerImpl()
    {
        LoggerCreator creator;
        creator(Tag::name, cLevel);
        m_logger = spdlog::get(Tag::name); // NOLINT(cppcoreguidelines-prefer-member-initializer)
    }

    /// Returns the only instance of the ModuleLoggerImpl.
    /// @return Instance of the ModuleLoggerImpl.
    /// @note The underlying logger object is created once during first usage.
    static ModuleLoggerImpl& instance()
    {
        static ModuleLoggerImpl object;
        return object;
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

/// Represents the logger, that has the module name as part of its type (via Tag).
/// @tparam Tag             Tag type, that will be used to obtain the module name (requires const char* name member).
/// @tparam level           Default logging level used by this logger.
/// @tparam LoggerCreator   Default creator used to create and register the logger.
/// @note This is a partial specialization for spdlog::level::off which should completely skip spdlog functions and
///       hopefully fully optimize calls when optimizations are enabled.
template <typename Tag, typename LoggerCreator>
class ModuleLoggerImpl<Tag, spdlog::level::off, LoggerCreator> {
public:
    /// Outputs the TRACE logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void trace(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Outputs the DEBUG logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void debug(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Outputs the INFO logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void info(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Outputs the WARN logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void warn(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Outputs the ERROR logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void error(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Outputs the CRITICAL logs.
    /// @tparam Args        Logger arguments' types.
    template <typename... Args>
    static void critical(std::string_view /*unused*/, Args&&... /*unused*/)
    {}

    /// Returns the underlying spdlog logger.
    /// @return Underlying spdlog logger.
    static std::shared_ptr<spdlog::logger> get() { return {}; }
};

/// Represents the logger, that has the module name as part of its type (via Tag).
/// @tparam Tag             Tag type, that will be used to obtain the module name (requires const char* name member).
/// @tparam level           Default logging level used by this logger.
/// @tparam LoggerCreator   Default creator used to create and register the logger.
/// @note In order to create/register a new module logger use the following code as template (name of the tag struct as
///       well as the logger type can be of any name):
///
/// 1) Default logger (with 'TRACE' level):
///      struct Mcp2515Tag { static constexpr auto* name = "MCP2515"; };
///      using Mcp2515Logger = logger::ModuleLogger<Mcp2515Tag>;
///
///      Mcp2515Logger::info("Some info message.");
///
/// 2) Default logger with explicitly set 'CRITICAL' level:
///      struct GpioTag { static constexpr auto* name = "GPIO"; };
///      using GpioLogger = logger::ModuleLogger<GpioTag, spdlog::level::critical>;
///
///      GpioLogger::info("Some info message.");
///
/// 3) Custom logger with explicitly set 'INFO' level:
///
///      struct CustomFlashLoggerCreator {
///          void operator()(const std::string& name, spdlog::level::level_enum level)
///          {
///              // Create and register custom logger here.
///          }
///      };
///
///      struct FlashTag { static constexpr auto* name = "FLASH"; };
///      using FlashLogger = logger::ModuleLogger<FlashTag, spdlog::level::info, CustomFlashLoggerCreator>;
///
///      FlashLogger::info("Some info message.");
///
template <typename Tag,
          spdlog::level::level_enum cLevel = spdlog::level::off,
          typename LoggerCreator = detail::DefaultLoggerCreator>
using ModuleLogger = ModuleLoggerImpl<Tag, cLevel, LoggerCreator>;

} // namespace utils::logger
