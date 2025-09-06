#pragma once
#include "io/base/registrator.h"

namespace kosio::io {
namespace detail {
class Send : public IoRegistrator<Send> {
public:
    Send(int sockfd, const void *buf, size_t len, int flags)
        : IoRegistrator{io_uring_prep_send, sockfd, buf, len, flags} {}

    auto await_resume() const noexcept -> Result<std::size_t, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
           return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};

class SendZC : public IoRegistrator<SendZC> {
public:
    SendZC(int sockfd, const void *buf, size_t len, int flags, unsigned zc_flags)
        : IoRegistrator{io_uring_prep_send_zc, sockfd, buf, len, flags, zc_flags} {}

    auto await_resume() const noexcept -> Result<std::size_t, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto send(int sockfd, const void *buf, size_t len, int flags) {
    return detail::Send{sockfd, buf, len, flags};
}
} // namespace kosio::io