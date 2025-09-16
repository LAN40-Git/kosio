#pragma once
#include "kosio/io/io.hpp"

namespace kosio::net::detail {
class Socket : public io::detail::FD {
public:
    explicit Socket(int fd)
        : FD(fd) {}

public:
    template <typename Addr>
        requires is_socket_address<Addr>
    [[nodiscard]]
    auto bind(const Addr &addr) -> Result<void> {
        if (::bind(fd_, addr.sockaddr(), addr.length()) != 0) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return {};
    }

    [[nodiscard]]
    auto listen(int maxn) const -> Result<void> {
        if (::listen(fd_, maxn) != 0) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return {};
    }

    [[REMEMBER_CO_AWAIT]]
    auto shutdown(int how) const noexcept {
        return io::detail::Shutdown{fd_, how};
    }

public:
    template <typename T = Socket>
    [[nodiscard]]
    static auto create(const int domain, const int type, const int protocol) -> Result<T> {
        auto fd = ::socket(domain, type, protocol);
        if (fd < 0) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return T{Socket{fd}};
    }
};
} // namespace kosio::net::detail