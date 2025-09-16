#pragma once
#include "kosio/net/addr.hpp"
#include "kosio/net/datagram.hpp"

namespace kosio::net {
class UdpDatagram : public detail::BaseDatagram<UdpDatagram, SocketAddr>,
                    public detail::ImplBoradcast<UdpDatagram>,
                    public detail::ImplTTL<UdpDatagram> {
public:
    explicit UdpDatagram(detail::Socket &&inner)
        : BaseDatagram{std::move(inner)} {}

public:
    [[nodiscard]]
    static auto unbound(bool is_ipv6 = false) -> Result<UdpDatagram> {
        return detail::Socket::create<UdpDatagram>(is_ipv6 ? AF_INET6 : AF_INET,
                                                   SOCK_DGRAM | SOCK_NONBLOCK,
                                                   IPPROTO_UDP);
    }
};
} // namespace kosio::net