#pragma once
#include "socket/socket.h"
#include "socket/impl/impl_local_addr.h"
#include "socket/impl/impl_peer_addr.h"
#include "socket/impl/impl_stream_read.h"
#include "socket/impl/impl_stream_write.h"

namespace coruring::socket::detail
{
template<class Stream, class Addr>
class BaseStream : public ImplStreamRead<BaseStream<Stream, Addr>>,
                   public ImplStreamWrite<BaseStream<Stream, Addr>>,
                   public ImplLocalAddr<BaseStream<Stream, Addr>, Addr>,
                   public ImplPeerAddr<BaseStream<Stream, Addr>, Addr> {
public:
    explicit BaseStream(Socket &&inner)
        : inner_(std::move(inner)) {}

protected:
    [[REMEMBER_CO_AWAIT]]
    auto shutdown(int how) noexcept {
        return inner_.shutdown(how);
    }

    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept {
        return inner_.close();
    }

    [[nodiscard]]
    auto fd() const noexcept {
        return inner_.fd();
    }

public:
    [[REMEMBER_CO_AWAIT]]
    static auto connect(const Addr& addr) {
        class Connect : public io::detail::IoRegistrator<Connect> {
        private:
            using Super = io::detail::IoRegistrator<Connect>;
        public:
            Connect(const Addr& addr)
                : Super{io_uring_prep_connect, -1, nullptr, sizeof(Addr)}
                , addr_(addr) {}

            auto await_suspend(std::coroutine_handle<> handle) -> bool {
                fd_ = ::socket(addr_.family(), SOCK_STREAM | SOCK_NONBLOCK, 0);
                if (fd_ < 0) [[unlikely]] {
                    this->cb_.result_ = errno;
                    io_uring_prep_nop(this->sqe_);
                    io_uring_sqe_set_data(this->sqe_, nullptr);
                    return false;
                }
                this->sqe_->fd = fd_;
                this->sqe_->addr = static_cast<unsigned long>(addr_.sockaddr());
                Super::await_suspend(handle);
                return true;
            }

            auto await_resume() noexcept -> std::expected<Stream, std::error_code> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return Stream{Socket{fd_}};
                } else {
                    if (fd_ >= 0) {
                        ::close(fd_);
                    }
                    return std::unexpected{std::error_code(-this->cb_.result_, std::system_category())};
                }
            }

        private:
            int fd_;
            Addr addr_;
        };
        return Connect{addr};
    }

private:
    Socket inner_;
};
}
