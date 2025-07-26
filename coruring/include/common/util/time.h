#pragma once
#include <ctime>
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace coruring::util {
constexpr auto CACHE_THRESHOLD = std::chrono::milliseconds(1);

static inline uint64_t current_ms() noexcept {
    struct alignas(64) Cache {
        timespec last_ts {};
        int64_t cached_ms = 0;
    };
    static thread_local Cache tls_cache;

    timespec now{};
    clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
    if (now.tv_sec == tls_cache.last_ts.tv_sec &&
        (now.tv_nsec - tls_cache.last_ts.tv_nsec) < 1'000'000) { // 1ms阈值
        return tls_cache.cached_ms;
        }

    tls_cache.last_ts = now;
    tls_cache.cached_ms = now.tv_sec * 1000 + now.tv_nsec / 1'000'000;
    return tls_cache.cached_ms;
}
} // namespace coruring::util