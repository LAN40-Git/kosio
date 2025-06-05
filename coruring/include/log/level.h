#pragma once

#include <string_view>
#include <utility>

namespace coruring::log
{
enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

// ANSI 颜色代码
namespace ansi {
    constexpr std::string_view Reset   = "\033[0m";
    constexpr std::string_view Black   = "\033[30m";
    constexpr std::string_view Red     = "\033[31m";
    constexpr std::string_view Green   = "\033[32m";
    constexpr std::string_view Yellow  = "\033[33m";
    constexpr std::string_view Blue    = "\033[34m";
    constexpr std::string_view Magenta = "\033[35m";
    constexpr std::string_view Cyan    = "\033[36m";
    constexpr std::string_view White   = "\033[37m";
}

[[nodiscard]]
consteval auto level_to_string(LogLevel level) noexcept -> std::string_view {
    using enum LogLevel;
    switch (level) {
        case Debug:  return "[Debug]";
        case Info:   return "[Info]";
        case Warn:   return "[Warn]";
        case Error:  return "[Error]";
        case Fatal:  return "[Fatal]";
        default:     return "[Unknown]";
    }
}

} // namespace coruring::log
