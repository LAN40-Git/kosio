#pragma once
#include <ctime>
#include <atomic>
#include <chrono>
#include <stdexcept>

namespace kosio::util {
constexpr std::chrono::nanoseconds CACHE_THRESHOLD{1'000'000}; // 1ms

static inline uint64_t current_ms() noexcept {
    struct alignas(64) Cache {
        timespec last_ts {};
        uint64_t cached_ms = 0;
    };
    thread_local Cache tls_cache;

    timespec now{};
    clock_gettime(CLOCK_MONOTONIC_COARSE, &now);

    uint64_t now_ms = static_cast<uint64_t>(now.tv_sec) * 1000ULL + now.tv_nsec / 1'000'000;
    uint64_t cached_ms = tls_cache.cached_ms;

    if (now_ms >= cached_ms) {
        uint64_t diff_ms = now_ms - cached_ms;
        if (diff_ms < CACHE_THRESHOLD.count() / 1'000'000) {
            return cached_ms;
        }
    }

    tls_cache.last_ts = now;
    tls_cache.cached_ms = now_ms;
    return now_ms;
}
} // namespace kosio::util