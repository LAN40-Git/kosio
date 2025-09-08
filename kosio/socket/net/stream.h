#pragma once
#include "kosio/socket/net/addr.h"
#include "kosio/socket/stream.h"

namespace kosio::socket::net
{
class TcpStream : public detail::BaseStream<TcpStream, SocketAddr> {
public:
    explicit TcpStream(detail::Socket &&inner)
        : BaseStream(std::move(inner)) {}
};
} // namespace kosio::socket::net
