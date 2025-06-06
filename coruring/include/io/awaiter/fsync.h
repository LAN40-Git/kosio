#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Fsync : public IoRegistrator<Fsync> {
public:
    Fsync(int fd, unsigned fsync_flags)
        : IoRegistrator{io_uring_prep_fsync, fd, fsync_flags} {}

    auto await_resume() noexcept -> std::expected<void, std::error_code> {
        IoUring::callback_map().erase(&this->cb_);
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};

[[REMEMBER_CO_AWAIT]]
static inline auto fsync(int fd, unsigned fsync_flags) {
    return Fsync{fd, fsync_flags};
}
} // namespace coruring::io