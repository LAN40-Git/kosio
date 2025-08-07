#include "runtime/timer/wheel/level.h"

void coruring::runtime::timer::wheel::detail::Level::add_entry(std::unique_ptr<Entry> entry, uint64_t remaining_ms) {
    auto slot = slot_for(remaining_ms);
    slots_[slot].push_back(std::move(entry));
}

auto coruring::runtime::timer::wheel::detail::Level::slot_for(uint64_t remaining_ms) const noexcept -> std::size_t {
    auto move_slot = remaining_ms / PRECISION[level_];
    return (current_slot + move_slot) & SLOT_MASK;
}
