#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Recv : public IoRegistrator<Recv> {
    public:
        Recv(int sockfd, void *buf, size_t len, int flags)
            : IoRegistrator{io_uring_prep_recv, sockfd, buf, len, flags} {}

        auto await_resume() noexcept -> std::expected<std::size_t, std::error_code> {
            detail::IoUring::callback_map().erase(&this->cb_);
            if (this->cb_.result_ >= 0) [[likely]] {
                return static_cast<std::size_t>(this->cb_.result_);
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto recv(int sockfd, void *buf, size_t len, int flags) {
    return detail::Recv{sockfd, buf, len, flags};
}
}