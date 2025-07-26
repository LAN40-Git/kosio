#include "runtime/timer/timer.h"

auto coruring::runtime::Timer::add_entry(io::detail::Callback* data,
    uint64_t expiration_ms) -> std::expected<detail::Entry*, std::error_code> {
    uint64_t current_ms = current_ms();
    if (expiration_ms <= current_ms) [[unlikely]] {
        return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
             std::system_category()}};
    }
    auto remaining_ms = expiration_ms - current_ms;
    if (remaining_ms >=
        detail::Wheel<detail::MAX_LEVEL, detail::SLOTS>::PRECISION[detail::MAX_LEVEL-1])
        [[unlikely]] {
            return std::unexpected{std::error_code{static_cast<int>(std::errc::invalid_argument),
                 std::system_category()}};
        }

    auto entry = std::make_unique<detail::Entry>(detail::Entry{data, expiration_ms});
    return wheel_.add_entry(std::move(entry), (remaining_ms));
}
