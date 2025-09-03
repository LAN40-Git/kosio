#pragma once

#include <string_view>
#include <utility>

namespace coruring::log::detail {
enum class LogLevel {
    Verbose,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

[[nodiscard]]
consteval auto level_to_string(LogLevel level) noexcept -> std::string_view {
    using enum LogLevel;
    switch (level) {
        case Verbose: return "Verbose";
        case Debug:  return "Debug";
        case Info:   return "Info";
        case Warn:   return "[Warn";
        case Error:  return "Error";
        case Fatal:  return "Fatal";
        default:
            std::unreachable();
            return "UNKNOWN LOG LEVEL";
    }
}

    [[nodiscard]]
consteval auto level_to_color(LogLevel level) noexcept -> std::string_view {
    switch (level) {
        using enum LogLevel;
        case Verbose:
        return "\033[97;46m"; // cyan
        case Debug:
        return "\033[97;44m"; // blue
        case Info:
        return "\033[97;42m"; // green
        case Warn:
        return "\033[90;43m"; // yellow
        case Error:
        return "\033[97;41m"; // red
        case Fatal:
        return "\033[97;45m"; // purple
        default:
        std::unreachable();
        return "NOT DEFINE COLOR";
    }
}

[[nodiscard]]
consteval auto reset_format() noexcept -> std::string_view {
    return "\033[0m";
}
} // namespace coruring::log
