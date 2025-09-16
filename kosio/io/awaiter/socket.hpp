#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class Socket : public IoRegistrator<Socket> {
public:
    Socket(int domain, int type, int protocol, unsigned int flags)
        : IoRegistrator(io_uring_prep_socket, domain, type, protocol, flags) {}

    auto await_resume() const noexcept -> Result<int> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return this->cb_.result_;
        } else {
            return std::unexpected{make_error(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto socket(int domain, int type, int protocol, unsigned int flags) {
    return detail::Socket{domain, type, protocol, flags};
}
} // namespace kosio::io