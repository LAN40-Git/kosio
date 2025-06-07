#pragma once
#include "socket/listener.h"
#include "stream.h"

namespace coruring::socket::net
{
class TcpListener : public detail::BaseListener<TcpListener, TcpStream, SocketAddr> {
public:
    explicit TcpListener(detail::Socket &&inner)
        : BaseListener(std::move(inner)) {}
};
}
