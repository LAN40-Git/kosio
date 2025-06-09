#pragma once
#include <cstdint>
#include <ctime>
#include <stdexcept>

namespace coruring::util
{
constexpr static inline uint64_t CACHE_THRESHOLD_NS = 1'000'000; // 1ms缓存阈值

// 计算毫秒时间戳（防溢出）
static uint64_t calculate_ms(const timespec& ts) noexcept {
    return static_cast<uint64_t>(ts.tv_sec) * 1'000 +
           static_cast<uint64_t>(ts.tv_nsec) / 1'000'000;
}

static inline uint64_t current_ms() {
    timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        throw std::runtime_error("clock_gettime failed");
    }

    thread_local uint64_t last_cached = 0;
    thread_local timespec last_ts = {0, 0};

    // 仅当秒级变化或纳秒差超过阈值时更新缓存
    if (ts.tv_sec != last_ts.tv_sec ||
        (ts.tv_nsec - last_ts.tv_nsec) >= CACHE_THRESHOLD_NS) {
        last_ts = ts;
        last_cached = calculate_ms(ts);
        }
    return last_cached;
}
}
