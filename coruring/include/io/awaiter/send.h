#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
class Send : public IoRegistrator<Send> {
public:
    Send(int sockfd, const void *buf, size_t len, int flags)
        : IoRegistrator{io_uring_prep_send, sockfd, buf, len, flags} {}

    auto await_resume() noexcept -> std::expected<std::size_t, std::error_code> {
        IoUring::callback_map().erase(&this->cb_);
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};

[[REMEMBER_CO_AWAIT]]
static inline auto send(int sockfd, const void *buf, size_t len, int flags) {
    return Send{sockfd, buf, len, flags};
}
}