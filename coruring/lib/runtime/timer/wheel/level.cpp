#include "runtime/timer/wheel/level.h"

void coruring::runtime::timer::wheel::detail::Level::add_entry(std::unique_ptr<Entry> entry, uint64_t remaining_ms) {
    auto slot = slot_for(remaining_ms);
    slots_[slot].push_back(std::move(entry));
    occupied_ |= (1ULL << slot);
}

auto coruring::runtime::timer::wheel::detail::Level::next_expiration_time()
const noexcept -> std::optional<uint64_t> {
    if (occupied_ == 0) return std::nullopt;

    uint64_t masked = occupied_ & ~((1ULL << current_slot) - 1);
    if (masked == 0) masked = occupied_;

    std::size_t next_slot = __builtin_ctzll(masked);
    auto ticks = (next_slot - current_slot) & SLOT_MASK;
    return ticks * PRECISION[level_];
}

auto coruring::runtime::timer::wheel::detail::Level::slot_for(uint64_t remaining_ms)
const noexcept -> std::size_t {
    auto move_slot = remaining_ms / PRECISION[level_];
    return (current_slot + move_slot) & SLOT_MASK;
}
