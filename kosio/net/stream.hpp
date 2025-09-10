#pragma once
#include "kosio/net/socket.hpp"
#include "kosio/net/impl/impl_stream_read.hpp"
#include "kosio/net/impl/impl_stream_write.hpp"
#include "kosio/net/impl/impl_peer_addr.hpp"
#include "kosio/net/impl/impl_local_addr.hpp"

namespace kosio::net::detail {
template <class Stream, class Addr>
class BaseStream : public ImplStreamRead<BaseStream<Stream, Addr>>
                 , public ImplStreamWrite<BaseStream<Stream, Addr>>
                 , public ImplLocalAddr<BaseStream<Stream, Addr>, Addr>
                 , public ImplPeerAddr<BaseStream<Stream, Addr>, Addr> {
protected:
    explicit BaseStream(Socket &&inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto shutdown(int how) const noexcept {
        return inner_.shutdown(how);
    }

    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept {
        return inner_.close();
    }

    [[nodiscard]]
    auto fd() const noexcept -> int {
        return inner_.fd();
    }

private:
    Socket inner_;
};
} // namespace kosio::net::detail