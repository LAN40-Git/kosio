#include "runtime/timer/wheel/wheel.h"

coruring::runtime::timer::wheel::Wheel::Wheel() {
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        levels_[level] = std::make_unique<detail::Level>(level);
    }
}

auto coruring::runtime::timer::wheel::Wheel::insert(std::unique_ptr<Entry> entry, uint64_t when)
const noexcept -> Result<Entry *, TimerError> {
    if (when <= elapsed_) {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }

    auto level = level_for(when);

    auto* entry_ptr = entry.get();
    levels_[level]->add_entry(std::move(entry), when);
    return entry_ptr;
}

auto coruring::runtime::timer::wheel::Wheel::next_expiration_time()
const noexcept -> std::optional<uint64_t> {
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        if (auto expiration_time = levels_[level]->next_expiration_time(elapsed_)) {
            return expiration_time;
        }
    }
    return std::nullopt;
}

auto coruring::runtime::timer::wheel::Wheel::handle_expired_entries(uint64_t now) {

}

auto coruring::runtime::timer::wheel::Wheel::level_for(uint64_t when)
    noexcept -> std::size_t {
    std::size_t level = 0;
    while (when >= LEVEL_RANGE[level]) {
        level++;
    }

    return level;
}
