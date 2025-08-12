#pragma once
#include "common/error.h"
#include "runtime/io/io_uring.h"
#include "io/base/callback.h"
#include <optional>
#include <list>

namespace coruring::runtime::timer {
namespace detail {
static constexpr auto compute_slot_range() -> std::array<uint64_t, runtime::detail::NUM_LEVELS> {
    std::array<uint64_t, runtime::detail::NUM_LEVELS> slot_range{};
    slot_range[0] = 1;
    for (auto i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        slot_range[i] = slot_range[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return slot_range;
}

static constexpr auto compute_level_range() -> std::array<uint64_t, runtime::detail::NUM_LEVELS> {
    std::array<uint64_t, runtime::detail::NUM_LEVELS> level_range{};
    level_range[0] = 64;
    for (auto i = 1; i < runtime::detail::NUM_LEVELS; i++) {
        level_range[i] = level_range[i - 1] * runtime::detail::LEVEL_MULT;
    }
    return level_range;
}
} // namespace detail

// 时间轮最大时间跨度
static constexpr uint64_t MAX_DURATION = (1ULL << (6 * runtime::detail::NUM_LEVELS)) - 1;

// 每层槽位对应的时间跨度
static constexpr auto SLOT_RANGE = detail::compute_slot_range();

// 每层对应的时间跨度
static constexpr auto LEVEL_RANGE = detail::compute_level_range();

class Entry {
public:
    Entry() = default;
    explicit Entry(coruring::io::detail::Callback *data, uint64_t expiration_time)
        : data_{data}
        , expiration_time_{expiration_time} {}

public:
    void execute() const;

public:
    coruring::io::detail::Callback *data_{};            // 提交到 io_uring 的数据
    uint64_t                        expiration_time_{}; // 绝对超时时间（毫秒）
};

struct Expiration {
    std::size_t level;
    std::size_t slot;
    uint64_t deadline;
};

using EntryList = std::list<std::unique_ptr<Entry>>;
using Slots = std::array<EntryList, runtime::detail::LEVEL_MULT>;
} // namespace coruring::runtime::timer
