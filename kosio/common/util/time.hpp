#pragma once
#include <ctime>
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace kosio::util {

static inline uint64_t current_ms() noexcept {
    timespec now{};
    clock_gettime(CLOCK_REALTIME_COARSE, &now);
    return static_cast<uint64_t>(now.tv_sec) * 1000ULL + now.tv_nsec / 1'000'000;
}

static inline std::string format_time(uint64_t ms) {
    thread_local std::array<char, 64> buffer{};
    thread_local time_t               last_second{0};

    auto cur_second = static_cast<time_t>(ms / 1000);
    auto cur_millisecond = ms % 1000;
    if (cur_second != last_second) {
        struct tm tm_time{};
        ::localtime_r(&cur_second, &tm_time);
        constexpr auto format = "%Y-%m-%d %H:%M:%S";
        ::strftime(buffer.data(), buffer.size(), format, &tm_time);
    }

    return std::format("{}.{:03}", buffer.data(), cur_millisecond);
}
} // namespace kosio::util