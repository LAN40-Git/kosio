#pragma once
#include "runtime/timer/entry.h"

namespace coruring::runtime::timer::wheel::detail {
class Level {
public:
    explicit Level(std::size_t level)
        : level_(level) {}

public:
    void add_entry(std::unique_ptr<Entry> entry, uint64_t remaining_ms);

private:
    [[nodiscard]]
    auto slot_for(uint64_t remaining_ms) const noexcept -> std::size_t;

private:
    std::size_t          level_;
    // 最低有效位表示时隙零
    uint64_t             occupied_{};
    std::size_t          current_slot{0};
    // 槽位，用于存储任务队列
    timer::detail::Slots slots_{};
};
} // namespace coruring::runtime::timer::wheel::detail
