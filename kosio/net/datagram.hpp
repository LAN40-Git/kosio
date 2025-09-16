#pragma once
#include "kosio/net/socket.hpp"
#include "kosio/net/impl/impl_sockopt.hpp"
#include "kosio/net/impl/impl_peer_addr.hpp"
#include "kosio/net/impl/impl_local_addr.hpp"
#include "kosio/net/impl/impl_datagram_recv.hpp"
#include "kosio/net/impl/impl_datagram_send.hpp"

namespace kosio::net::detail {
    template <class Datagram, class Addr>
class BaseDatagram : public ImplAsyncSend<BaseDatagram<Datagram, Addr>, Addr>,
                     public ImplAsyncRecv<BaseDatagram<Datagram, Addr>, Addr>,
                     public ImplLocalAddr<BaseDatagram<Datagram, Addr>, Addr>,
                     public ImplPeerAddr<BaseDatagram<Datagram, Addr>, Addr> {
protected:
    explicit BaseDatagram(Socket &&inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto connect(const Addr &addr) noexcept {
        return io::detail::Connect{fd(), addr.sockaddr(), addr.length()};
    }

    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept {
        return inner_.close();
    }

    [[nodiscard]]
    auto fd() const noexcept {
        return inner_.fd();
    }

public:
    [[nodiscard]]
    static auto bind(const Addr &addr) -> Result<Datagram> {
        auto ret = Socket::create<Datagram>(addr.family(), SOCK_DGRAM | SOCK_NONBLOCK, 0);
        if (!ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        if (::bind(ret.value().fd(), addr.sockaddr(), addr.length()) != 0) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return Datagram{std::move(ret.value())};
    }

private:
    Socket inner_;
};
} // nanamespace kosio::net::detail