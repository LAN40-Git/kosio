#pragma once
#include "kosio/net/socket.hpp"
#include "kosio/net/impl/impl_local_addr.hpp"

namespace kosio::net::detail {
template <class Listener, class Stream, class Addr>
class BaseListener : public ImplLocalAddr<Listener, Addr> {
protected:
    explicit BaseListener(Socket&& inner)
        : inner_{std::move(inner)} {}

public:
    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept {
        return inner_.close();
    }

    [[REMEMBER_CO_AWAIT]]
    auto fd() const noexcept -> int {
        return inner_.fd();
    }

    [[REMEMBER_CO_AWAIT]]
    auto accept() const noexcept {
        class Accept : public io::detail::IoRegistrator<Accept> {

        public:
            Accept(int fd)
                : io::detail::IoRegistrator<Accept>{io_uring_prep_accept,
                        fd,
                        reinterpret_cast<sockaddr *>(&addr_),
                        &length_,
                        SOCK_NONBLOCK} {}

            auto await_resume() const noexcept -> Result<std::pair<Stream, Addr>> {
                if (this->cb_.result_ >= 0) [[likely]] {
                    return std::make_pair(Stream{Socket{this->cb_.result_}}, addr_);
                } else {
                    return std::unexpected{make_error(-this->cb_.result_)};
                }
            }

        private:
            Addr      addr_{};
            socklen_t length_{sizeof(Addr)};
        };
        return Accept{fd()};
    }

public:
    [[nodiscard]]
    static auto bind(const Addr& addr) -> Result<Listener> {
        auto ret = Socket::create(addr.family(), SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (!ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        auto& inner = ret.value();
        if (auto ret = inner.bind(addr); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        if (auto ret = inner.listen(SOMAXCONN); !ret) [[unlikely]] {
            return std::unexpected{ret.error()};
        }
        return Listener{std::move(inner)};
    }

    [[nodiscard]]
    static auto bind(const std::span<Addr> &addresses) -> Result<Listener> {
        for (const auto &address : addresses) {
            if (auto ret = bind(address); ret) [[likely]] {
                return ret;
            } else {
                LOG_ERROR("Bind {} failed, error: {}", address.to_string(), ret.error().message());
            }
        }
        return std::unexpected{make_error(Error::kInvalidAddresses)};
    }

private:
    Socket inner_;
};
} // namespace kosio::net::detail