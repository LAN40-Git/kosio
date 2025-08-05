#include "runtime/timer/wheel/wheel.h"

auto coruring::runtime::timer::wheel::Wheel::insert(std::unique_ptr<Entry> entry,
    uint64_t when) noexcept -> Result<Entry *, TimerError> {
    if (when <= elapsed_) {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }

    auto level = level_for(when);

    auto* entry_ptr = entry.get();
    levels_[level].add_entry(std::move(entry), when);
    return entry_ptr;
}

void coruring::runtime::timer::wheel::Wheel::remove(Entry *entry, uint64_t when) noexcept {

}

auto coruring::runtime::timer::wheel::Wheel::level_for(uint64_t when) const noexcept -> std::size_t {

}
