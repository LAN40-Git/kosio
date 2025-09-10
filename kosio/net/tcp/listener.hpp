#pragma once
#include "kosio/net/tcp/stream.hpp"
#include "kosio/net/listener.hpp"

namespace kosio::net {
class TcpListener : public detail::BaseListener<TcpListener, TcpStream, SocketAddr>,
                    public detail::ImplTTL<TcpListener> {
public:
    explicit TcpListener(detail::Socket&& inner)
        : BaseListener{std::move(inner)} {}
};
} // namespace kosio::net