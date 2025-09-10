#pragma once
#include "kosio/io/awaiter/recv.hpp"

namespace kosio::net::detail {
template <class T, class Addr>
class ImplAsyncRecv {
public:
    [[REMEMBER_CO_AWAIT]]
    auto recv(std::span<char> buf, int flags = 0) const noexcept {
        return io::detail::Recv{static_cast<const T *>(this)->fd(),
                                buf.data(),
                                buf.size_bytes(),
                                flags};
    }

    [[REMEMBER_CO_AWAIT]]
    auto peek(std::span<char> buf) const noexcept {
        return this->recv(buf, MSG_PEEK);
    }

    [[REMEMBER_CO_AWAIT]]
    auto recv_from(std::span<char> buf, unsigned flags = 0) const noexcept {
        class RecvFrom : public io::detail::IoRegistrator<RecvFrom> {
        private:
            using Super = io::detail::IoRegistrator<RecvFrom>;

        public:
            RecvFrom(int fd, std::span<char> buf, unsigned flags)
                : Super{io_uring_prep_recvmsg, fd, &msg_, flags}
            , iovecs_{.iov_base = buf.data(), .iov_len = buf.size_bytes()}
            , msg_{.msg_name = &addr_,
                   .msg_namelen = sizeof(addr_),
                   .msg_iov = &iovecs_,
                   .msg_iovlen = 1,
                   .msg_control = nullptr,
                   .msg_controllen = 0,
                   .msg_flags = static_cast<int>(flags)} {};

            auto await_resume() const noexcept -> Result<std::pair<std::size_t, Addr>, IoError> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return std::make_pair(static_cast<std::size_t>(this->cb_.result_), addr_);
                } else {
                    return std::unexpected{make_error<IoError>(-this->cb_.result_)};
                }
            }

        private:
            iovec  iovecs_;
            Addr   addr_{};
            msghdr msg_ {};
        };
        return RecvFrom{static_cast<const T *>(this)->fd(), buf, flags};
    }

    [[REMEMBER_CO_AWAIT]]
    auto peek_from(std::span<char> buf) const noexcept {
        return this->recv_from(buf, MSG_PEEK);
    }
};
} // namespace kosio::net::detail
