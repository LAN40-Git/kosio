#pragma once
#include "io/base/registrator.h"

namespace kosio::io {
namespace detail {
class FSetXattr : public IoRegistrator<FSetXattr> {
public:
    FSetXattr(int fd, const char *name, const char *value, int flags, unsigned int len)
        : IoRegistrator{io_uring_prep_fsetxattr, fd, name, value, flags, len} {}

    auto await_resume() const noexcept -> Result<std::size_t, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto
fsetxattr(int fd, const char *name, const char *value, int flags, unsigned int len) {
    return detail::FSetXattr{fd, name, value, flags, len};
}
} // namespace kosio::io