#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class Fsync : public IoRegistrator<Fsync> {
public:
    Fsync(int fd, unsigned fsync_flags)
        : IoRegistrator{io_uring_prep_fsync, fd, fsync_flags} {}

    auto await_resume() const noexcept -> Result<void, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto fsync(int fd, unsigned fsync_flags) {
    return detail::Fsync{fd, fsync_flags};
}
} // namespace coruring::io