#include "runtime/timer/timer.h"

auto coruring::runtime::timer::Timer::insert(io::detail::Callback* data,
    uint64_t expiration_time) noexcept -> std::expected<Entry*, std::error_code> {
    if (expiration_time <= start_) [[unlikely]] {
        return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
             std::system_category()}};
    }
    auto remaining_ms = expiration_time - start_;
    if (remaining_ms >= wheel::Wheel::MAX_DURATION) [[unlikely]] {
            return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
                 std::system_category()}};
    }
    auto entry = std::make_unique<Entry>(Entry{data, expiration_time});
    return wheel_.insert(std::move(entry), remaining_ms);
}

void coruring::runtime::timer::Timer::remove(Entry* entry) noexcept {
    if (entry->expiration_time_ <= start_) [[unlikely]] {
        return;
    }

    wheel_.remove(entry, entry->expiration_time_ - start_);
}
