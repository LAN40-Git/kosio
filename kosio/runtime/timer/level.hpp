#pragma once
#include "kosio/runtime/config.hpp"
#include "kosio/runtime/timer/entry.hpp"

namespace kosio::runtime::timer::detail {
class Level {
public:
    explicit Level(std::size_t level)
        : level_(level) {}

public:
    void add_entry(std::unique_ptr<Entry>&& entry, uint64_t when) {
        auto slot = slot_for(when);
        slots_[slot].push_back(std::move(entry));
        occupied_ |= (1ULL << slot);
    }

    [[nodiscard]]
    auto next_expiration(uint64_t now)
    const noexcept -> std::optional<Expiration> {
        auto slot = next_occupied_slot(now);
        if (!slot) {
            return std::nullopt;
        }

        // 获取当前层级对齐的起始时间（LEVEL_RANGE[level_] 是 2 的幂）
        // now & !(LEVEL_RANGE[level_] - 1) 清除 now 的低 log2(LEVEL_RANGE[level_])位
        auto level_start = now & ~(LEVEL_RANGE[level_] - 1);
        // 计算非空槽位内事件到期时间的最小值（相对于分层时间轮启动）
        // 此最小值表示的是槽位内最早到期的事件对应的时间，比如：
        // 当前在第 1 层，slot = next_occupied_slot(now) = 1
        // 分层时间轮自启动算起，经过 deadline 时间后，第 1 层的 第 1 槽位中
        // 可能有事件到期（注意并非全部）
        auto deadline = level_start + slot.value() * SLOT_RANGE[level_];
        if (deadline <= now) {
            // 这种情况可能在最高层回绕发生
            deadline += LEVEL_RANGE[level_];
        }

        return Expiration{ level_, slot.value(), deadline};
    }

    [[nodiscard]]
    auto next_occupied_slot(uint64_t now) const noexcept -> std::optional<std::size_t> {
        if (occupied_ == 0) {
            return std::nullopt;
        }

        // 根据时间轮运行的时间来计算当前所在槽位
        auto now_slot = now / SLOT_RANGE[level_];
        // 将位图右移到当前槽位
        auto occupied = std::rotr(occupied_, static_cast<int>(now_slot));
        // 从当前槽位开始（包含当前槽位），查找非空槽位，位图对应值为 1 表示当前槽位非空
        // 通过计算 0 的数量来寻找非空槽位，例如：
        // 找到 0 个 0 => 当前槽位非空（对应下标 0 + now_slot）
        // 找到 1 个 0 => 当前槽位往后的一个槽位非空（对应下表 1 + now_slot）
        auto zeros = std::countr_zero(occupied);
        // 计算非空槽位的绝对位置
        auto slot = (zeros + now_slot) % runtime::detail::LEVEL_MULT;

        return slot;
    }

    [[nodiscard]]
    auto take_slot(std::size_t slot) -> EntryList {
        EntryList entries;
        std::swap(entries, slots_[slot]);
        occupied_ &= ~(1ULL << slot);
        return entries;
    }

private:
    [[nodiscard]]
    auto slot_for(uint64_t when)
    const noexcept -> std::size_t {
        return (when >> (level_ * 6)) % runtime::detail::LEVEL_MULT;
    }

private:
    // 当前层级
    std::size_t level_;
    // 最低有效位表示时隙零
    uint64_t    occupied_{};
    // 槽位，用于存储任务队列
    Slots       slots_{};
};
} // namespace kosio::runtime::timer::detail
