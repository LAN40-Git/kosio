#pragma once
#include "kosio/common/error.hpp"
#include <sys/socket.h>

namespace kosio::net::detail {
template <class T, class Addr>
struct ImplLocalAddr {
    [[nodiscard]]
    auto local_addr() const noexcept -> Result<Addr, IoError> {
        Addr      addr{};
        socklen_t len{sizeof(addr)};
        if (::getsockname(static_cast<const T *>(this)->fd(), addr.sockaddr(), &len) == -1)
            [[unlikely]] {
                return std::unexpected{make_error<IoError>(errno)};
            }
        return addr;
    }
};
} // namespace kosio::net::detail
