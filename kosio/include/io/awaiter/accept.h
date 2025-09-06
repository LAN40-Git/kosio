#pragma once
#include "io/base/registrator.h"

namespace kosio::io {
namespace detail {
class Accept : public IoRegistrator<Accept> {
public:
    Accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags)
        : IoRegistrator{io_uring_prep_accept, fd, addr, addrlen, flags} {}

    auto await_resume() const noexcept -> Result<int, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return this->cb_.result_;
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto accept(int fd, sockaddr* addr, socklen_t* addrlen, int flags) {
    return detail::Accept{fd, addr, addrlen, flags};
}
} // namespace kosio::io