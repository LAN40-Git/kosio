#pragma once
#include "socket/listener.h"

namespace coruring::socket::net
{
class TcpListener : public BaseListener<TcpListener, SocketAddr> {
public:
    explicit TcpListener(Socket &&inner)
        : BaseListener(std::move(inner)) {}
};
}
