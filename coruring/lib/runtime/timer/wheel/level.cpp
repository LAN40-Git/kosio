#include "runtime/timer/wheel/level.h"
#include <bit>

void coruring::runtime::timer::wheel::detail::Level::add_entry(std::unique_ptr<Entry> entry, uint64_t when) {
    auto slot = slot_for(when);
    slots_[slot].push_back(std::move(entry));
    occupied_ |= (1ULL << slot);
}

auto coruring::runtime::timer::wheel::detail::Level::next_expiration_time(uint64_t now)
const noexcept -> std::optional<uint64_t> {
    auto slot = next_occupied_slot(now);
    if (!slot) {
        return std::nullopt;
    }

    // 这里其实就是减法
    auto level_start = now & !(LEVEL_RANGE[level_] - 1);

    auto deadline = level_start + slot.value() * SLOT_RANGE[level_];
    if (deadline <= now) {
        // 当 now 刚好等于 LEVEL_RANGE[level_] 时可能出现这种情况
        // 比如：now = 64ms = LEVEL_RANGE[0]
        // level_start = 0，那么 deadline 就会小于等于 now
        // 此时只需要给 deadline 加上一个 LEVEL_RANGE[level_] 即可
        deadline += LEVEL_RANGE[level_];
    }

    return deadline;
}

auto coruring::runtime::timer::wheel::detail::Level::next_occupied_slot(uint64_t now)
const noexcept -> std::optional<std::size_t> {
    if (occupied_ == 0) {
        return std::nullopt;
    }

    auto now_slot = now / SLOT_RANGE[level_];
    auto occupied = std::rotr(occupied_, static_cast<int>(now_slot));
    auto zeros = std::countr_zero(occupied);
    auto slot = (zeros + now_slot) % runtime::detail::LEVEL_MULT;

    return slot;
}

auto coruring::runtime::timer::wheel::detail::Level::take_slot(std::size_t slot) -> timer::detail::EntryList {
    timer::detail::EntryList entries;
    std::swap(entries, slots_[slot]);
    occupied_ &= ~(1ULL << slot);
    return entries;
}

auto coruring::runtime::timer::wheel::detail::Level::slot_for(uint64_t duration)
const noexcept -> std::size_t {
    // 这里不需要取模，因为 wheel 保证 duration 一定在当前层级的时间跨度内
    return (duration >> (level_ * runtime::detail::NUM_LEVELS));
}
