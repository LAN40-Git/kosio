#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class SendMsg : public IoRegistrator<SendMsg> {
private:
    using Super = IoRegistrator<SendMsg>;

public:
    SendMsg(int fd, const struct msghdr *msg, unsigned flags)
        : Super{io_uring_prep_sendmsg, fd, msg, flags} {}

    auto await_resume() const noexcept -> Result<std::size_t> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return ::std::unexpected{make_error(-this->cb_.result_)};
        }
    }
};

class SendMsgZC : public IoRegistrator<SendMsgZC> {
private:
    using Super = IoRegistrator<SendMsgZC>;

public:
    SendMsgZC(int fd, const struct msghdr *msg, unsigned flags)
        : Super{io_uring_prep_sendmsg_zc, fd, msg, flags} {}

    auto await_resume() const noexcept -> Result<std::size_t> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return ::std::unexpected{make_error(-this->cb_.result_)};
        }
    }
};
} // namespace detail
[[REMEMBER_CO_AWAIT]]
static inline auto sendmsg(int fd, const struct msghdr *msg, unsigned flags) {
    return detail::SendMsg{fd, msg, flags};
}

[[REMEMBER_CO_AWAIT]]
static inline auto sendmsg_zc(int fd, const struct msghdr *msg, unsigned flags) {
    return detail::SendMsgZC{fd, msg, flags};
}
} // namespace kosio::io
