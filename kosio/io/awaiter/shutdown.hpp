#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class Shutdown : public IoRegistrator<Shutdown> {
public:
    Shutdown(int fd, int how)
        : IoRegistrator{io_uring_prep_shutdown, fd, how} {}

    auto await_resume() const noexcept -> Result<void> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        } else {
            return std::unexpected{make_error(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto shutdown(int fd, int how) {
    return detail::Shutdown(fd, how);
}
} // namespace kosio::io