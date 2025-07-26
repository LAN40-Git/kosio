#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class Cancel : public IoRegistrator<Cancel> {
public:
    Cancel(int fd, int flags)
        : IoRegistrator{io_uring_prep_cancel_fd, fd, flags} {}

    auto await_resume() noexcept -> std::expected<void, std::error_code> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto cancel(int fd, int flags) {
    return detail::Cancel{fd, flags};
}
} // namespace coruring::io
