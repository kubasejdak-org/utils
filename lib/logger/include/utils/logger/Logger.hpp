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

#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

/// Registers custom user-defined logger with the given properties.
/// @tparam LoggerType      Type used to access given logger.
/// @param loggerName       Name of the logger (displayed as [<NAME>] in log message).
/// @param logLevel         Default logging level for this logger.
/// @param loggerCreator    Callable object which will create the logger.
#define REGISTER_LOGGER_EX(LoggerType, loggerName, logLevel, loggerCreator)                                            \
    struct LoggerType##Tag {                                                                                           \
        static constexpr auto* name = loggerName;                                                                      \
    };                                                                                                                 \
    using LoggerType = utils::logger::ModuleLogger<LoggerType##Tag, logLevel, loggerCreator>

#ifdef APPLICATION_LOGGER
    #if __has_include("ApplicationLogger.hpp")
        #include "ApplicationLogger.hpp"
    #endif

    #define LOGGER_CREATOR app::ApplicationLoggerCreator
#else
    #define LOGGER_CREATOR utils::logger::detail::DefaultLoggerCreator
#endif

/// Registers custom user-defined logger with the given properties.
/// @tparam LoggerType      Type used to access given logger.
/// @param loggerName       Name of the logger (displayed as [<NAME>] in log message).
/// @param logLevel         Default logging level for this logger.
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
    /// @param logLevel     Log level.
    /// @param fmt          Format string.
    /// @param args         Logger arguments.
    template <typename... Args>
    static void log(spdlog::level::level_enum logLevel, Args&&... args)
    {
        get()->log(logLevel, std::forward<Args>(args)...);
    }

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
        m_logger = spdlog::get(Tag::name);
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
    static void log(spdlog::level::level_enum /*unused*/, Args&&... /*unused*/)
    {}

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
