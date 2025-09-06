#pragma once
#include "kosio/include/socket/listener.h"
#include "kosio/include/socket/net/stream.h"

namespace kosio::socket::net
{
class TcpListener : public detail::BaseListener<TcpListener, TcpStream, SocketAddr> {
public:
    explicit TcpListener(detail::Socket &&inner)
        : BaseListener(std::move(inner)) {}
};
} // namespace kosio::socket::net
