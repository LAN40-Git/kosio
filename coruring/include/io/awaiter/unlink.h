#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Unlink : public IoRegistrator<Unlink> {
    public:
        Unlink(int dfd, const char *path, int flags)
            : IoRegistrator(io_uring_prep_unlinkat, dfd, path, flags) {}

        Unlink(const char *path, int flags)
            : IoRegistrator{io_uring_prep_unlink, path, flags} {}

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
static inline auto unlinkat(int dfd, const char *path, int flags) {
    return detail::Unlink(dfd, path, flags);
}

[[REMEMBER_CO_AWAIT]]
static inline auto unlink(const char *path, int flags) {
    return detail::Unlink{path, flags};
}
} // namespace coruring::io