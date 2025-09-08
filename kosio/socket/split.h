#pragma once
#include "kosio/socket/socket.h"
#include "kosio/socket/impl/impl_peer_addr.h"
#include "kosio/socket/impl/impl_local_addr.h"
#include "kosio/socket/impl/impl_stream_read.h"
#include "kosio/socket/impl/impl_stream_write.h"

namespace kosio::socket::detail {
template<class Addr>
class ReadHalf : public ImplStreamRead<ReadHalf<Addr>>,
                 public ImplLocalAddr<ReadHalf<Addr>, Addr>,
                 public ImplPeerAddr<ReadHalf<Addr>, Addr> {
public:
    ReadHalf(Socket &inner)
        : inner_(inner) {}

public:
    [[nodiscard]]
    auto fd() const noexcept -> int {
        return inner_.fd();
    }

private:
    Socket &inner_;
};

template<class Addr>
class WriteHalf : public ImplStreamWrite<WriteHalf<Addr>>,
                  public ImplLocalAddr<WriteHalf<Addr>, Addr>,
                  public ImplPeerAddr<WriteHalf<Addr>, Addr> {
public:
    WriteHalf(Socket &inner)
        : inner_(inner) {}

public:
    [[nodiscard]]
    auto fd() const noexcept -> int {
        return inner_.fd();
    }

private:
    Socket &inner_;
};

template <class T>
class OwnedBase {
protected:
    OwnedBase(std::shared_ptr<T> stream)
        : stream_(stream) {}

public:
    [[nodiscard]]
    auto fd() const noexcept -> int {
        return stream_->fd();
    }

    auto reunite(OwnedBase &other) -> std::expected<T, std::error_code> {
        if (this == std::addressof(other) || stream_ != other.stream_) {
            return std::unexpected{std::make_error_code(std::errc::invalid_argument)};
        }
        other.stream_.reset();
        std::shared_ptr<T> temp{nullptr};
        temp.swap(stream_);
        return std::move(*temp);
    }

protected:
    std::shared_ptr<T> stream_;
};

template <class T, class Addr>
class OwnedReadHalf : public OwnedBase<T>
                    , public ImplStreamRead<OwnedReadHalf<T, Addr>>
                    , public ImplLocalAddr<OwnedReadHalf<T, Addr>, Addr>
                    , public ImplPeerAddr<OwnedReadHalf<T, Addr>, Addr> {
public:
    OwnedReadHalf(std::shared_ptr<T> stream)
        : OwnedBase<T>(std::move(stream)) {}

    // ~OwnedReadHalf() {
    //     if (this->stream_) {
    //         // TODO register shutdown write
    //     }
    // }
};

template <class T, class Addr>
class OwnedWriteHalf : public OwnedBase<T>
                     , public ImplStreamWrite<OwnedWriteHalf<T, Addr>>
                     , public ImplLocalAddr<OwnedWriteHalf<T, Addr>, Addr>
                     , public ImplPeerAddr<OwnedWriteHalf<T, Addr>, Addr> {
public:
    OwnedWriteHalf(std::shared_ptr<T> stream)
        : OwnedBase<T>(std::move(stream)) {}

    // ~OwnedReadHalf() {
    //     if (this->stream_) {
    //         // TODO register shutdown write
    //     }
    // }
};
} // namespace kosio::socket::detail
