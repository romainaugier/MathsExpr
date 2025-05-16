// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_LOG)
#define __MATHSEXPR_LOG

#include "mathsexpr/mathsexpr.h"

#include <format>
#include <iostream>
#include <string_view>
#include <iterator>

MATHSEXPR_NAMESPACE_BEGIN

enum LogLevel : std::uint32_t
{
    Critical,
    Error,
    Warning,
    Info,
    Debug,
};

class Logger
{
    LogLevel _level;

    Logger() : _level(LogLevel::Info) {}
    ~Logger() {}

public:
    static Logger& get_instance() { static Logger l; return l; }

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
    }
};

template<typename... Args>
MATHSEXPR_FORCE_INLINE void log_critical(std::format_string<Args...> format, Args&&... args) noexcept 
{ 
    Logger::get_instance().log(mathsexpr::LogLevel::Critical, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHSEXPR_FORCE_INLINE void log_error(std::format_string<Args...> format, Args&&... args) noexcept 
{ 
    Logger::get_instance().log(mathsexpr::LogLevel::Error, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHSEXPR_FORCE_INLINE void log_warning(std::format_string<Args...> format, Args&&... args) noexcept 
{ 
    Logger::get_instance().log(mathsexpr::LogLevel::Warning, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHSEXPR_FORCE_INLINE void log_info(std::format_string<Args...> format, Args&&... args) noexcept
{ 
    Logger::get_instance().log(mathsexpr::LogLevel::Info, format, std::forward<Args>(args)...);
}

template<typename... Args>
MATHSEXPR_FORCE_INLINE void log_debug(std::format_string<Args...> format, Args&&... args) noexcept 
{
    Logger::get_instance().log(mathsexpr::LogLevel::Debug, format, std::forward<Args>(args)...);
}

MATHSEXPR_FORCE_INLINE void set_log_level(const LogLevel level) noexcept
{ 
    Logger::get_instance().set_level(level);
}

MATHSEXPR_NAMESPACE_END

template <>
struct std::formatter<mathsexpr::LogLevel> : std::formatter<std::string_view> 
{
    auto format(const mathsexpr::LogLevel level, format_context& ctx) const 
    {
        std::string_view name;

        switch(level) 
        {
            case mathsexpr::LogLevel::Critical:
                name = "CRITICAL"; 
                break;
            case mathsexpr::LogLevel::Error:
                name = "ERROR";
                break;
            case mathsexpr::LogLevel::Warning:
                name = "WARNING";
                break;
            case mathsexpr::LogLevel::Info:
                name = "INFO";
                break;
            case mathsexpr::LogLevel::Debug:
                name = "DEBUG";
                break;
            default:
                name = "UNKNOWN";
                break;
        }

        return formatter<std::string_view>::format(name, ctx);
    }
};

#endif /* !defined(__MATHSEXPR_LOG) */