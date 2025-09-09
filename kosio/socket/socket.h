#pragma once
#include "kosio/io/io.hpp"
#include "kosio/common/error.hpp"

namespace kosio::socket::detail {
/**Support
 * IPv4
 * -----END
 */
class Socket : public io::detail::FD {
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
    auto listen(int maxn = SOMAXCONN) -> std::expected<void, std::error_code>;
    [[REMEMBER_CO_AWAIT]]
    auto shutdown(int how) noexcept;
    [[nodiscard]]
    auto set_reuseaddr(int option) -> std::expected<void, std::error_code>;
    [[nodiscard]]
    auto set_reuseport(int option) -> std::expected<void, std::error_code>;

public:
    template <typename T = Socket>
    [[nodiscard]]
    static auto create(int domain, int type, int protocol) ->
    std::expected<T, std::error_code> {
        auto fd = ::socket(domain, type, protocol);
        if (fd < 0) [[unlikely]] {
            return std::unexpected{std::error_code(errno, std::generic_category())};
        }
        return T{Socket{fd}};
    }
};
} // namespace kosio::socket::detail
