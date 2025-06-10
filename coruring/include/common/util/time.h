#pragma once
#include <stdexcept>

namespace coruring::util
{
constexpr static inline auto CACHE_THRESHOLD = std::chrono::milliseconds(1);

static inline int64_t current_ms() {
    using Clock = std::chrono::steady_clock;
    static thread_local auto last_cached = Clock::now();
    static thread_local int64_t last_value = 0;

    auto now = Clock::now();
    if (now - last_cached >= CACHE_THRESHOLD) {
        last_cached = now;
        last_value = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
    }
    return last_value;
}
}
