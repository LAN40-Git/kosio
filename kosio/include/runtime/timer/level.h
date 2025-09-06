#pragma once
#include "runtime/timer/entry.h"

namespace kosio::runtime::timer::detail {
class Level {
public:
    explicit Level(std::size_t level)
        : level_(level) {}

public:
    void add_entry(std::unique_ptr<Entry> entry, uint64_t when);
    [[nodiscard]]
    auto next_expiration(uint64_t now) const noexcept -> std::optional<Expiration>;
    [[nodiscard]]
    auto next_occupied_slot(uint64_t now) const noexcept -> std::optional<std::size_t>;
    [[nodiscard]]
    auto take_slot(std::size_t slot) -> EntryList;

private:
    [[nodiscard]]
    auto slot_for(uint64_t duration) const noexcept -> std::size_t;

private:
    // 当前层级
    std::size_t level_;
    // 最低有效位表示时隙零
    uint64_t    occupied_{};
    // 槽位，用于存储任务队列
    Slots       slots_{};
};
} // namespace kosio::runtime::timer::detail
