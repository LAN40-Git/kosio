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

auto coruring::runtime::timer::wheel::Wheel::next_expiration()
const noexcept -> std::optional<Expiration> {
    // 如果有需要到期的事件，则返回 0 表示有事件需要立即处理
    if (!pending_.empty()) {
        return Expiration{0, 0, elapsed_};
    }

    // 否则从第 0 层开始遍历获取到 最近一个非空事件所在位置
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        if (auto expiration = levels_[level]->next_expiration(elapsed_)) {
            return expiration;
        }
    }
    // 返回空表示可以无限睡眠直到被唤醒
    return std::nullopt;
}

void coruring::runtime::timer::wheel::Wheel::handle_expired_entries(uint64_t now) {
    elapsed_ = now;

    auto max_level = level_for(now);
    auto expiration = this->next_expiration();
    while (expiration) {
        auto entries = take_entries(expiration.value());

    }
}

auto coruring::runtime::timer::wheel::Wheel::take_entries(Expiration expiration)
const noexcept -> EntryList {
    return levels_[expiration.level]->take_slot(expiration.slot);
}

auto coruring::runtime::timer::wheel::Wheel::level_for(uint64_t when)
    noexcept -> std::size_t {
    std::size_t level = 0;
    while (when >= LEVEL_RANGE[level]) {
        level++;
    }

    return level;
}
