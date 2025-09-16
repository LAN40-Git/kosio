#pragma once
#include "kosio/common/error.hpp"
#include <sys/socket.h>

namespace kosio::net::detail {
template <class T, class Addr>
struct ImplPeerAddr {
    [[nodiscard]]
    auto peer_addr() const noexcept -> Result<Addr> {
        Addr      addr{};
        socklen_t len{sizeof(addr)};
        if (::getpeername(static_cast<const T *>(this)->fd(), addr.sockaddr(), &len) == -1)
            [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return addr;
    }
};
} // namespace kosio::net::detail
