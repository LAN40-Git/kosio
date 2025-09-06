#include "socket/net/socket.h"


auto kosio::socket::net::TcpSocket::bind(const SocketAddr& addr) noexcept
-> std::expected<void, std::error_code> {
    return inner_.bind<SocketAddr>(addr);
}

auto kosio::socket::net::TcpSocket::listen(int n)
-> std::expected<TcpListener, std::error_code> {
    if (auto ret = inner_.listen(n); !ret) [[unlikely]] {
        return std::unexpected{ret.error()};
    }
    return TcpListener{std::move(inner_)};
}

auto kosio::socket::net::TcpSocket::connect(const SocketAddr& addr) {
    class Connect : public io::detail::IoRegistrator<Connect> {
    public:
        Connect(Socket&& inner, const SocketAddr& addr)
            : IoRegistrator<Connect>{io_uring_prep_connect, inner.fd(), nullptr, addr.length()}
            , inner_(std::move(inner))
            , addr_(addr) {
            sqe_->addr = reinterpret_cast<unsigned long>(addr_.sockaddr());
        }

        auto await_resume() noexcept -> std::expected<void, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return {};
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }

    private:
        Socket     inner_;
        SocketAddr addr_;
    };
    return Connect{std::move(inner_), addr};
}

auto kosio::socket::net::TcpSocket::v4()
-> std::expected<TcpSocket, std::error_code> {
    return Socket::create<TcpSocket>(AF_INET, SOCK_STREAM |
                                        SOCK_NONBLOCK, IPPROTO_TCP);
}

auto kosio::socket::net::TcpSocket::v6()
-> std::expected<TcpSocket, std::error_code> {
    return Socket::create<TcpSocket>(AF_INET6, SOCK_STREAM |
                                        SOCK_NONBLOCK, IPPROTO_TCP);
}
