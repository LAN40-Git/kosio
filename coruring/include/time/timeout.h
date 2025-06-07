#pragma once
#include "io/base/registrator.h"

namespace coruring::time
{
namespace detail
{
    class Timeout : public io::detail::IoRegistrator<Timeout> {
    public:
        Timeout(__kernel_timespec *ts, unsigned count, unsigned flags)
            : IoRegistrator(io_uring_prep_timeout, ts, count, flags) {}

        auto await_resume() noexcept -> std::expected<void, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return {};
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto timeout(__kernel_timespec *ts, unsigned count, unsigned flags) {
    return detail::Timeout{ts, count, flags};
}
} // namespace coruring::time