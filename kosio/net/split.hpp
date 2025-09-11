#pragma once
#include "kosio/net/impl/impl_local_addr.hpp"
#include "kosio/net/impl/impl_peer_addr.hpp"
#include "kosio/net/impl/impl_stream_read.hpp"
#include "kosio/net/impl/impl_stream_write.hpp"
#include "kosio/net/socket.hpp"

namespace kosio::net::detail {
template <class Addr>
class ReadHalf : public ImplStreamRead<ReadHalf<Addr>>,
                 public ImplLocalAddr<ReadHalf<Addr>, Addr>,
                 public ImplPeerAddr<ReadHalf<Addr>, Addr> {
public:
    ReadHalf(Socket &inner)
        : inner_{inner} {}

public:
    [[nodiscard]]
    auto fd() const noexcept {
        return inner_.fd();
    }

private:
    Socket &inner_;
};

template <class Addr>
class WriteHalf : public ImplStreamWrite<WriteHalf<Addr>>,
                  public ImplLocalAddr<WriteHalf<Addr>, Addr>,
                  public ImplPeerAddr<WriteHalf<Addr>, Addr> {
public:
    WriteHalf(Socket &inner)
        : inner_{inner} {}

public:
    [[nodiscard]]
    auto fd() const noexcept {
        return inner_.fd();
    }

private:
    Socket &inner_;
};
} // namespace kosio::net::detail