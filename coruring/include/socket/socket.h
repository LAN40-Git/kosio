#pragma once
#include "io/io.h"

namespace coruring::socket
{
/**Support
 * IPv4
 * -----END
 */
class Socket : public io::FD {
public:
    explicit Socket(int fd)
        : FD(fd) {}

public:
    template<typename Addr>
        requires is_socket_address<Addr>
    [[nodiscard]]
    auto bind(const Addr &addr)
    -> std::expected<void, std::error_code> {
        if (::bind(fd_, addr.sockaddr(), addr.length()) != 0) [[unlikely]] {
            return std::unexpected{std::error_code{errno, std::generic_category()}};
        }
        return {};
    }
    [[nodiscard]]
    auto listen(int maxn = SOMAXCONN) -> std::expected<void, std::error_code> {
        if (::listen(fd_, maxn) != 0) [[unlikely]] {
            return std::unexpected{std::error_code{errno, std::generic_category()}};
        }
        return {};
    }
    [[nodiscard]]
    auto set_reuseaddr(int option) -> std::expected<void, std::error_code> {
        auto ret = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (ret == -1) [[unlikely]] {
            return std::unexpected{std::error_code{errno, std::generic_category()}};
        }
        return {};
    }

public:
    template <typename T = Socket>
    [[nodiscard]]
    static auto create(int domain, int type, int protocol) ->
    std::expected<Socket, std::error_code> {
        auto fd = ::socket(domain, type, protocol);
        if (fd < 0) [[unlikely]] {
            return std::unexpected{std::error_code(errno, std::generic_category())};
        }
        return T{Socket{fd}};
    }
};
}
