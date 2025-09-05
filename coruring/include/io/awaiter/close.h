#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class Close : public IoRegistrator<Close> {
public:
    Close(int fd)
        : IoRegistrator{io_uring_prep_close, fd} {}

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
static inline auto close(int fd) {
    return detail::Close {fd};
}
} // namespace coruring::io
