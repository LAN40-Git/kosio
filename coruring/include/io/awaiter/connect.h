#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Connect : public IoRegistrator<Connect> {
    public:
        Connect(int fd, sockaddr *addr, socklen_t addrlen)
            : IoRegistrator{io_uring_prep_connect, fd, addr, addrlen} {}

        auto await_resume() noexcept -> std::expected<void, std::error_code> {
            detail::IoUring::callback_map().erase(&this->cb_);
            if (this->cb_.result_ >= 0) [[likely]] {
                return {};
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto connect(int fd, sockaddr *addr, socklen_t addrlen) {
    return detail::Connect {fd, addr, addrlen};
}
}