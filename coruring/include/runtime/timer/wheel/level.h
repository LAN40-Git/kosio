#pragma once
#include "runtime/timer/entry.h"

namespace coruring::runtime::timer::wheel::detail {
class Level {
public:
    explicit Level(std::size_t level)
        : level_(level) {}

public:
    // 时间轮每层时间跨度
    static constexpr auto PRECISION = timer::detail::compute_precision();

public:
    void add_entry(std::unique_ptr<Entry> entry, uint64_t when);

private:
    auto slot_for(uint64_t remaining_ms) const noexcept -> std::size_t;

private:
    std::size_t          level_;
    // 最低有效位表示时隙零
    uint64_t             occupied_{};
    // 槽位，用于存储任务队列
    timer::detail::Slots slots_{};
};
} // namespace coruring::runtime::timer::wheel::detail
