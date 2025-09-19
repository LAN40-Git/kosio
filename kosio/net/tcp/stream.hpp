#pragma once
#include "kosio/net/addr.hpp"
#include "kosio/net/stream.hpp"
#include "kosio/net/impl/impl_sockopt.hpp"

namespace kosio::net {
class TcpStream : public detail::BaseStream<TcpStream, SocketAddr>,
                  public detail::ImplNodelay<TcpStream>,
                  public detail::ImplLinger<TcpStream>,
                  public detail::ImplTTL<TcpStream> {
public:
    explicit TcpStream(detail::Socket&& inner)
        : BaseStream{std::move(inner)} {}
};
using OwnedTcpStreamReader = detail::OwnedReadHalf<TcpStream, SocketAddr>;
using OwnedTcpStreamWriter = detail::OwnedWriteHalf<TcpStream, SocketAddr>;
} // namespace kosio::net
