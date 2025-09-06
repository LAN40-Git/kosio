#pragma once
#include <expected>
#include <system_error>
#include <sys/socket.h>

namespace kosio::socket::detail {
template<class T, class Addr>
struct ImplPeerAddr {
    [[nodiscard]]
    auto peer_addr() const noexcept -> std::expected<Addr, std::error_code> {
        Addr addr{};
        socklen_t addrlen{sizeof(addr)};
        if (::getpeername(static_cast<const T*>(this)->fd(),
            addr.sockaddr(), &addrlen) == -1) [[unlikely]] {
                return std::unexpected{std::error_code{errno, std::generic_category()}};
            }
        return addr;
    }
};
} // namespace kosio::socket::detail
