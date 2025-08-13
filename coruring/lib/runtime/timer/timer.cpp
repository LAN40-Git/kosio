#include "runtime/timer/timer.h"

coruring::runtime::timer::Timer::Timer() noexcept {
    t_timer = this;
}

coruring::runtime::timer::Timer::~Timer() {
    t_timer = nullptr;
}

auto coruring::runtime::timer::Timer::insert(coruring::io::detail::Callback* data
                                             , uint64_t expiration_time)
const noexcept -> Result<Entry*, TimerError> {
    if (expiration_time <= start_) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    auto when = expiration_time - start_;
    if (when >= MAX_DURATION) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kTooLongTime)};
    }
    auto entry = std::make_unique<Entry>(Entry{data, expiration_time});
    return wheel_.insert(std::move(entry), when);
}

void coruring::runtime::timer::Timer::remove(Entry* entry) noexcept {
    if (entry) {
        entry->data_ = nullptr;
    }
}

auto coruring::runtime::timer::Timer::next_expiration()
const noexcept -> std::optional<Expiration> {
    return wheel_.next_expiration();
}

void coruring::runtime::timer::Timer::poll() {
    wheel_.poll(util::current_ms() - start_);
}
