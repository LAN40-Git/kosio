#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class SetXattr : public IoRegistrator<SetXattr> {
private:
    using Super = IoRegistrator<SetXattr>;

public:
    SetXattr(const char *name, const char *value, const char *path, int flags, unsigned int len)
        : Super{io_uring_prep_setxattr, name, value, path, flags, len} {}

    auto await_resume() const noexcept -> Result<void, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        } else {
            return ::std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};

} // namespace detail
[[REMEMBER_CO_AWAIT]]
static inline auto
setxattr(const char *name, const char *value, const char *path, int flags, unsigned int len) {
    return detail::SetXattr{name, value, path, flags, len};
}
} // namespace kosio::io
