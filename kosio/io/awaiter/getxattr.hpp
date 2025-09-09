#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class GetXattr : public IoRegistrator<GetXattr> {
public:
    GetXattr(const char *name, char *value, const char *path, unsigned int len)
        : IoRegistrator{io_uring_prep_getxattr, name, value, path, len} {}

    auto await_resume() const noexcept -> Result<std::size_t, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto getxattr(const char *name, char *value, const char *path, unsigned int len) {
    return detail::GetXattr{name, value, path, len};
}
} // namespace kosio::io