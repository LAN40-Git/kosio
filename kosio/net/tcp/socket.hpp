#pragma once
#include "kosio/net/socket.hpp"
#include "kosio/net/tcp/stream.hpp"
#include "kosio/net/tcp/listener.hpp"

namespace kosio::net {
class TcpSocket {
    using Socket = detail::Socket;
public:
    explicit TcpSocket(detail::Socket&& inner)
        : inner_{std::move(inner)} {}

public:
    [[nodiscard]]
    auto bind(const SocketAddr& addr) {
        return inner_.bind<SocketAddr>(addr);
    }

    [[nodiscard]]
    auto listen(int n) -> Result<TcpListener> {
        if (auto ret = inner_.listen(n); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        return TcpListener{std::move(inner_)};
    }

    [[nodiscard]]
    [[REMEMBER_CO_AWAIT]]
    auto connect(const SocketAddr &addr) {
        class Awaiter : public io::detail::IoRegistrator<Awaiter> {
        public:
            Awaiter(Socket &&inner, const SocketAddr &addr)
                : IoRegistrator{io_uring_prep_connect, inner.fd(), nullptr, addr.length()}
            , inner_{std::move(inner)}
            , addr_{addr} {
                sqe_->addr = reinterpret_cast<unsigned long>(addr_.sockaddr());
            }

            auto await_resume() noexcept -> Result<TcpStream> {
                if (this->cb_.result_ < 0) [[unlikely]] {
                    return std::unexpected{make_error(-this->cb_.result_)};
                }
                return TcpStream{std::move(inner_)};
            }

        private:
            Socket     inner_;
            SocketAddr addr_;
        };
        return Awaiter{std::move(inner_), addr};
    }

    [[nodiscard]]
    auto fd() const noexcept {
        return inner_.fd();
    }

public:
    [[nodiscard]]
    static auto v4() -> Result<TcpSocket> {
        return Socket::create<TcpSocket>(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    }

    [[nodiscard]]
    static auto v6() -> Result<TcpSocket> {
        return Socket::create<TcpSocket>(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    }

private:
    Socket inner_;
};
} // namespace kosio::net