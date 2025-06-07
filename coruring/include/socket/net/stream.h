#pragma once
#include "socket/net/addr.h"
#include "socket/stream.h"

namespace coruring::socket::net
{
class TcpStream : public detail::BaseStream<TcpStream, SocketAddr> {
public:
    explicit TcpStream(detail::Socket &&inner)
        : BaseStream(std::move(inner)) {}
};
}
