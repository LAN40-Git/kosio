#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class FSetXattr : public IoRegistrator<FSetXattr> {
public:
    FSetXattr(int fd, const char *name, const char *value, int flags, unsigned int len)
        : IoRegistrator{io_uring_prep_fsetxattr, fd, name, value, flags, len} {}

    auto await_resume() const noexcept -> std::expected<std::size_t, std::error_code> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        }
        return ::std::unexpected{std::error_code{-this->cb_.result_,
        std::generic_category()}};
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto
fsetxattr(int fd, const char *name, const char *value, int flags, unsigned int len) {
    return detail::FSetXattr{fd, name, value, flags, len};
}
} // namespace coruring::io