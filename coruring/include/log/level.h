#pragma once

#include <string_view>
#include <utility>

namespace coruring::log::detail {
enum class LogLevel {
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
        case Debug:  return "[Debug]";
        case Info:   return "[Info]";
        case Warn:   return "[Warn]";
        case Error:  return "[Error]";
        case Fatal:  return "[Fatal]";
        default:     return "[Unknown]";
    }
}
} // namespace coruring::log
