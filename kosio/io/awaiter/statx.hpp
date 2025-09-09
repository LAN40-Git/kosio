#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
    namespace detail {
        class Statx : public IoRegistrator<Statx> {
        public:
            Statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf)
                : IoRegistrator{io_uring_prep_statx, dfd, path, flags, mask, statxbuf} {}

            auto await_resume() const noexcept -> Result<void, IoError> {
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
    statx(int dfd, const char *path, int flags, unsigned mask, struct statx *statxbuf) {
        return detail::Statx{dfd, path, flags, mask, statxbuf};
    }
} // namespace kosio::io