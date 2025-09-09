#pragma once
#include "kosio/net/socket.hpp"
#include "kosio/net/impl/impl_stream_read.hpp"
#include "kosio/net/impl/impl_stream_write.hpp"

namespace kosio::net::detail {
template <class Stream, class Addr>
class BaseStream : ImplStreamRead<BaseStream<Stream, Addr>>
                 , ImplStreamWrite<BaseStream<Stream, Addr>> {
protected:
    explicit BaseStream(Socket &&inner)
        : inner_{std::move(inner)} {}

private:
    Socket inner_;
};
} // namespace kosio::net::detail