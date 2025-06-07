#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Statx : public IoRegistrator<Statx> {
    public:
        Statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf)
            : IoRegistrator{io_uring_prep_statx, dfd, path, flags, mask, statxbuf} {}

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
static inline auto
statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf) {
    return detail::Statx{dfd, path, flags, mask, statxbuf};
}
} // namespace coruring::io