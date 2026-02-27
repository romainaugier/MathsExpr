// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_LOG)
#define __MATHEXPR_LOG

#include "mathexpr/common.hpp"

#include <format>
#include <iostream>
#include <string_view>
#include <iterator>

MATHEXPR_NAMESPACE_BEGIN

enum LogLevel : std::uint32_t
{
    Critical,
    Error,
    Warning,
    Info,
    Debug,
};

class MATHEXPR_API Logger
{
    LogLevel _level;

    Logger() : _level(LogLevel::Info) {}
    ~Logger() {}

public:
    static Logger& get_instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void set_level(const LogLevel level) { this->_level = level; }

    template<typename... Args>
    void log(const LogLevel level, std::format_string<Args...> format, Args&&... args) const noexcept
    {
        if(static_cast<std::uint32_t>(level) > static_cast<std::uint32_t>(this->_level))
        {
            return;
        }

        static std::ostream_iterator<char> out(std::cout);

        std::format_to(out, "[{}] {}\n", level, std::vformat(format.get(), std::make_format_args(args...)));
        std::cout.flush();
    }
};

template<typename... Args>
MATHEXPR_FORCE_INLINE void log_critical(std::format_string<Args...> format, Args&&... args) noexcept
{
    Logger::get_instance().log(mathexpr::LogLevel::Critical, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHEXPR_FORCE_INLINE void log_error(std::format_string<Args...> format, Args&&... args) noexcept
{
    Logger::get_instance().log(mathexpr::LogLevel::Error, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHEXPR_FORCE_INLINE void log_warning(std::format_string<Args...> format, Args&&... args) noexcept
{
    Logger::get_instance().log(mathexpr::LogLevel::Warning, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHEXPR_FORCE_INLINE void log_info(std::format_string<Args...> format, Args&&... args) noexcept
{
    Logger::get_instance().log(mathexpr::LogLevel::Info, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHEXPR_FORCE_INLINE void log_debug(std::format_string<Args...> format, Args&&... args) noexcept
{
    Logger::get_instance().log(mathexpr::LogLevel::Debug, format, std::forward<Args>(args)...);
}

MATHEXPR_FORCE_INLINE void set_log_level(const LogLevel level) noexcept
{
    Logger::get_instance().set_level(level);
}

MATHEXPR_NAMESPACE_END

template <>
struct std::formatter<mathexpr::LogLevel> : std::formatter<std::string_view>
{
    auto format(const mathexpr::LogLevel level, format_context& ctx) const
    {
        std::string_view name;

        switch(level)
        {
            case mathexpr::LogLevel::Critical:
                name = "CRITICAL";
                break;
            case mathexpr::LogLevel::Error:
                name = "ERROR";
                break;
            case mathexpr::LogLevel::Warning:
                name = "WARNING";
                break;
            case mathexpr::LogLevel::Info:
                name = "INFO";
                break;
            case mathexpr::LogLevel::Debug:
                name = "DEBUG";
                break;
            default:
                name = "UNKNOWN";
                break;
        }

        return formatter<std::string_view>::format(name, ctx);
    }
};

#endif /* !defined(__MATHEXPR_LOG) */
