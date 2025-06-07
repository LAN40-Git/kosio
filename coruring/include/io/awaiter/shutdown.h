#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Shutdown : public IoRegistrator<Shutdown> {
    public:
        Shutdown(int fd, int how)
            : IoRegistrator{io_uring_prep_shutdown, fd, how} {}

        auto await_resume() noexcept -> std::expected<void, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return {};
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
                                                   std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto shutdown(int fd, int how) {
    return detail::Shutdown(fd, how);
}
}