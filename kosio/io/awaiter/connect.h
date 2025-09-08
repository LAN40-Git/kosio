#pragma once
#include "kosio/io/base/registrator.h"

namespace kosio::io {
namespace detail {
class Connect : public IoRegistrator<Connect> {
public:
    Connect(int fd, sockaddr *addr, socklen_t addrlen)
        : IoRegistrator{io_uring_prep_connect, fd, addr, addrlen} {}

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
static inline auto connect(int fd, sockaddr *addr, socklen_t addrlen) {
    return detail::Connect {fd, addr, addrlen};
}
} // namespace kosio::io