#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Socket : public IoRegistrator<Socket> {
    public:
        Socket(int domain, int type, int protocol, unsigned int flags)
            : IoRegistrator(io_uring_prep_socket, domain, type, protocol, flags) {}

        auto await_resume() noexcept -> std::expected<int, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return this->cb_.result_;
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto socket(int domain, int type, int protocol, unsigned int flags) {
    return detail::Socket{domain, type, protocol, flags};
}
} // namespace coruring::io