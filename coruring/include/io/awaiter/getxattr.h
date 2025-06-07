#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class GetXattr : public IoRegistrator<GetXattr> {
    public:
        GetXattr(const char *name, char *value, const char *path, unsigned int len)
            : IoRegistrator{io_uring_prep_getxattr, name, value, path, len} {}

        auto await_resume() const noexcept -> std::expected<std::size_t, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return static_cast<std::size_t>(this->cb_.result_);
            }
            return std::unexpected{std::error_code(-this->cb_.result_,
            std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto getxattr(const char *name, char *value, const char *path, unsigned int len) {
    return detail::GetXattr{name, value, path, len};
}
} // namespace coruring::io