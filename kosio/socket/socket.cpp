#include "kosio/socket/socket.h"

auto kosio::socket::detail::Socket::listen(int maxn) -> std::expected<void, std::error_code> {
    if (::listen(fd_, maxn) != 0) [[unlikely]] {
        return std::unexpected{std::error_code{errno, std::generic_category()}};
    }
    return {};
}

auto kosio::socket::detail::Socket::shutdown(int how) noexcept {
    return io::detail::Shutdown{fd_, how};
}

auto kosio::socket::detail::Socket::set_reuseaddr(int option) -> std::expected<void, std::error_code> {
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) [[unlikely]] {
        return std::unexpected{std::error_code{errno, std::generic_category()}};
    }
    return {};
}

auto kosio::socket::detail::Socket::set_reuseport(int option) -> std::expected<void, std::error_code> {
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option)) == -1) [[unlikely]] {
        return std::unexpected{std::error_code{errno, std::generic_category()}};
    }
    return {};
}
