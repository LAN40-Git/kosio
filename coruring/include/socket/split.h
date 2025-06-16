#pragma once
#include "socket/socket.h"
#include "socket/impl/impl_peer_addr.h"
#include "socket/impl/impl_local_addr.h"
#include "socket/impl/impl_stream_recv.h"
#include "socket/impl/impl_stream_send.h"

namespace coruring::socket::detail
{
template<class Addr>
class RecvHalf : public ImplStreamRecv<RecvHalf<Addr>>,
                 public ImplLocalAddr<RecvHalf<Addr>, Addr>,
                 public ImplPeerAddr<RecvHalf<Addr>, Addr> {
public:
    RecvHalf(Socket &inner)
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
class SendHalf : public ImplStreamSend<SendHalf<Addr>>,
                  public ImplLocalAddr<SendHalf<Addr>, Addr>,
                  public ImplPeerAddr<SendHalf<Addr>, Addr> {
public:
    SendHalf(Socket &inner)
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
class OwnedRecvHalf : public OwnedBase<T>
                    , public ImplStreamRecv<OwnedRecvHalf<T, Addr>>
                    , public ImplLocalAddr<OwnedRecvHalf<T, Addr>, Addr>
                    , public ImplPeerAddr<OwnedRecvHalf<T, Addr>, Addr> {
public:
    OwnedRecvHalf(std::shared_ptr<T> stream)
        : OwnedBase<T>(std::move(stream)) {}

    // ~OwnedReadHalf() {
    //     if (this->stream_) {
    //         // TODO register shutdown write
    //     }
    // }
};

template <class T, class Addr>
class OwnedSendHalf : public OwnedBase<T>,
                       public ImplStreamSend<OwnedSendHalf<T, Addr>>,
                       public ImplLocalAddr<OwnedSendHalf<T, Addr>, Addr>,
                       public ImplPeerAddr<OwnedSendHalf<T, Addr>, Addr> {
public:
    OwnedSendHalf(std::shared_ptr<T> stream)
        : OwnedBase<T>(std::move(stream)) {}

    // ~OwnedReadHalf() {
    //     if (this->stream_) {
    //         // TODO register shutdown write
    //     }
    // }
};

}
