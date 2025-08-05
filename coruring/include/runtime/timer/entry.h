#pragma once
#include "common/error.h"
#include "runtime/io/io_uring.h"
#include "io/base/callback.h"
#include <optional>
#include <list>

namespace coruring::runtime::timer {
namespace detail {
// 编译期计算出每层时间轮的时间跨度
static constexpr auto compute_precision() {
    std::array<std::size_t, runtime::detail::NUM_LEVELS> precision{};
    precision[0] = 64;
    for (std::size_t i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        precision[i] = precision[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return precision;
}
} // namespace detail

// 时间轮最大时间跨度
static constexpr uint64_t MAX_DURATION = (1ULL << (6 * runtime::detail::NUM_LEVELS)) - 1;

// 掩码，X & MASK = X % LEVEL_MULT
static constexpr std::size_t SLOT_MASK = runtime::detail::LEVEL_MULT - 1;

// 位移数，X >> SHIFT = X / LEVEL_MULT
static constexpr std::size_t SHIFT = std::countr_zero(runtime::detail::LEVEL_MULT);

class Entry {
public:
    Entry() = default;
    explicit Entry(io::detail::Callback *data, uint64_t expiration_time)
        : data_{data}
        , expiration_time_{expiration_time} {}

public:
    void execute() const;

public:
    io::detail::Callback *data_{};            // 提交到 io_uring 的数据
    uint64_t              expiration_time_{}; // 绝对超时时间（毫秒）
};

namespace detail {
// Only used in wheel
using EntryList = std::list<std::unique_ptr<Entry>>;
using Slots = std::array<EntryList, runtime::detail::LEVEL_MULT>;
} // namespace detail
} // namespace coruring::runtime::timer
