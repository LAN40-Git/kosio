#include "runtime/timer/timer.h"

auto coruring::runtime::timer::Timer::insert(io::detail::Callback* data,
    uint64_t expiration_time) noexcept -> Result<Entry*, TimerError> {
    if (expiration_time <= start_) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    auto when = expiration_time - start_;
    if (when >= MAX_DURATION) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    auto entry = std::make_unique<Entry>(Entry{data, expiration_time});
    return wheel_.insert(std::move(entry), when);
}

void coruring::runtime::timer::Timer::remove(Entry* entry) noexcept {
    if (entry->expiration_time_ <= start_) [[unlikely]] {
        return;
    }

    wheel_.remove(entry, entry->expiration_time_ - start_);
}
