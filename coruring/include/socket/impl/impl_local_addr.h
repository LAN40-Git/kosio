#pragma once
#include <expected>
#include <system_error>
#include <sys/socket.h>

namespace coruring::socket::detail {
template<class T, class Addr>
struct ImplLocalAddr {
    [[nodiscard]]
    auto local_addr() const noexcept -> std::expected<Addr, std::error_code> {
        Addr addr{};
        socklen_t addrlen{sizeof(addr)};
        if (::getsockname(static_cast<const T*>(this)->fd(),
            addr.sockaddr(), &addrlen) == -1) [[unlikely]] {
            return std::unexpected{std::error_code{errno, std::generic_category()}};
        }
        return addr;
    }
};
} // namespace coruring::socket::detail
