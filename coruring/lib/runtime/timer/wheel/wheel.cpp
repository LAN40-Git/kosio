#include "runtime/timer/wheel/wheel.h"

coruring::runtime::timer::wheel::Wheel::Wheel() {
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        levels_[level] = std::make_unique<detail::Level>(level);
    }
}

auto coruring::runtime::timer::wheel::Wheel::insert(std::unique_ptr<Entry> entry,
                                                    uint64_t when) const noexcept -> Result<Entry *, TimerError> {
    if (when <= elapsed_) {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }

    auto remaining_ms = when - elapsed_;
    auto level = level_for(remaining_ms);

    auto* entry_ptr = entry.get();
    levels_[level]->add_entry(std::move(entry), remaining_ms);
    return entry_ptr;
}

void coruring::runtime::timer::wheel::Wheel::remove(Entry *entry) noexcept {
    // 标记为已完成
    if (entry) {
        entry->data_ = nullptr;
    }
}

auto coruring::runtime::timer::wheel::Wheel::level_for(uint64_t remaining_ms) const noexcept -> std::size_t {
    std::size_t level = 0;
    while (remaining_ms >= PRECISION[level]) {
        level++;
    }

    return level;
}
