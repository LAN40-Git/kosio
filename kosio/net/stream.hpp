#pragma once
#include "kosio/net/split.hpp"

namespace kosio::net::detail {
template <class Stream, class Addr>
class BaseStream : public ImplStreamRead<BaseStream<Stream, Addr>>
                 , public ImplStreamWrite<BaseStream<Stream, Addr>>
                 , public ImplLocalAddr<BaseStream<Stream, Addr>, Addr>
                 , public ImplPeerAddr<BaseStream<Stream, Addr>, Addr> {
public:
    using Reader = ReadHalf<Addr>;
    using Writer = WriteHalf<Addr>;

protected:
    explicit BaseStream(Socket &&inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto shutdown(int how) const noexcept {
        return inner_.shutdown(how);
    }

    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept {
        return inner_.close();
    }

    [[nodiscard]]
    auto fd() const noexcept -> int {
        return inner_.fd();
    }

public:
    [[REMEMBER_CO_AWAIT]]
    static auto connect(const Addr &addr) {
        class Connect : public io::detail::IoRegistrator<Connect> {
        private:
            using Super = io::detail::IoRegistrator<Connect>;

        public:
            Connect(const Addr &addr)
                : Super{io_uring_prep_connect, -1, nullptr, sizeof(Addr)}
            , addr_{addr} {}

            auto await_suspend(std::coroutine_handle<> handle) -> bool {
                fd_ = ::socket(addr_.family(), SOCK_STREAM | SOCK_NONBLOCK, 0);
                if (fd_ < 0) [[unlikely]] {
                    this->cb_.result_ = errno;
                    io_uring_prep_nop(this->sqe_);
                    io_uring_sqe_set_data(this->sqe_, nullptr);
                    return false;
                }
                this->sqe_->fd = fd_;
                this->sqe_->addr = reinterpret_cast<unsigned long>(addr_.sockaddr());
                Super::await_suspend(handle);
                return true;
            }

            auto await_resume() noexcept -> Result<Stream, IoError> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return Stream{Socket{fd_}};
                } else {
                    if (fd_ >= 0) {
                        ::close(fd_);
                    }
                    return std::unexpected{make_error<IoError>(-this->cb_.result_)};
                }
            }

        private:
            int  fd_;
            Addr addr_;
        };
        return Connect{addr};
    }

    [[nodiscard]]
    auto split() const noexcept -> std::pair<Reader, Writer> {
        return std::make_pair(Reader{inner_}, Writer{inner_});
    }

private:
    Socket inner_;
};
} // namespace kosio::net::detail