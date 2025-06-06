#pragma once
#include <netinet/in.h>
#include "socket.h"
#include "net/addr.h"

namespace coruring::socket
{
template <class Listener, class Addr>
class BaseListener {
protected:
    explicit BaseListener(Socket &&inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto accept() noexcept {
        class Accept : public io::IoRegistrator<Accept> {
        public:
            Accept(int fd)
                : io::IoRegistrator<Accept>{io_uring_prep_accept, fd, reinterpret_cast<sockaddr *>(&addr_), &addrlen_, 0} {}

            auto await_resume() const noexcept -> std::expected<std::pair<Socket, Addr>, std::error_code> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return std::make_pair(Socket{this->cb_.result_}, addr_);
                }
                return std::unexpected{std::error_code{-this->cb_.result_, std::generic_category()}};
            }

        private:
            Addr      addr_{};
            socklen_t addrlen_{sizeof(Addr)};
        };
        return Accept{fd()};
    }
    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept -> io::Close { return inner_.close(); }
    [[nodiscard]]
    auto fd() const noexcept -> int { return inner_.fd(); }

public:
    [[nodiscard]]
    static auto bind(const Addr& addr) -> std::expected<Listener, std::error_code> {
        auto ret = Socket::create(addr.family(), SOCK_STREAM, 0);
        if (!ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        auto &inner = ret.value();
        if (auto ret = inner.bind(addr); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        if (auto ret = inner.set_reuseaddr(1); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        if (auto ret = inner.listen(); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        return Listener{std::move(inner)};
    }
    
private:
    Socket inner_;
};
}
