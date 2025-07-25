#pragma once
#include "socket/impl/impl_local_addr.h"
#include "socket/socket.h"

namespace coruring::socket::detail
{
template <class Listener, class Stream, class Addr>
class BaseListener : public ImplLocalAddr<BaseListener<Listener, Stream, Addr>, Addr> {
protected:
    explicit BaseListener(Socket&& inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto accept() noexcept {
        class Accept : public io::detail::IoRegistrator<Accept> {
        public:
            explicit Accept(int fd)
                : io::detail::IoRegistrator<Accept>{io_uring_prep_accept, fd, reinterpret_cast<sockaddr *>(&addr_), &addrlen_, 0} {}

            auto await_resume() const noexcept -> std::expected<std::pair<Stream, Addr>, std::error_code> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return std::make_pair(Stream{Socket{this->cb_.result_}}, addr_);
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
    auto close() noexcept { return inner_.close(); }
    [[nodiscard]]
    auto fd() const noexcept -> int { return inner_.fd(); }

public:
    [[nodiscard]]
    static auto bind(const Addr& addr) -> std::expected<Listener, std::error_code> {
        auto ret = Socket::create(addr.family(), SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (!ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        auto &inner = ret.value();
        if (auto ret = inner.bind(addr); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        // 默认复用地址和端口
        if (auto ret = inner.set_reuseaddr(1); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        if (auto ret = inner.set_reuseport(1); !ret) [[unlikely]] {
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
} // namespace coruring::socket::net
