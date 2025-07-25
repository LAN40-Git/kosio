#pragma once
#include "listener.h"
#include "addr.h"
#include "stream.h"
#include "socket/socket.h"

namespace coruring::socket::net
{
class TcpSocket {
    using Socket = detail::Socket;

public:
    explicit TcpSocket(detail::Socket&& inner)
        : inner_(std::move(inner)) {}

public:
    [[nodiscard]]
    auto bind(const SocketAddr& addr) noexcept -> std::expected<void, std::error_code>;
    [[nodiscard]]
    auto listen(int n) -> std::expected<TcpListener, std::error_code>;
    [[REMEMBER_CO_AWAIT]]
    auto connect(const SocketAddr& addr);
    [[nodiscard]]
    auto fd() const noexcept { return inner_.fd(); }

public:
    [[nodiscard]]
    static auto v4() -> std::expected<TcpSocket, std::error_code>;
    [[nodiscard]]
    static auto v6() -> std::expected<TcpSocket, std::error_code>;

private:
    Socket inner_;
};
} // namespace coruring::socket::net