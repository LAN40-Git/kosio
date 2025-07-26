#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class MkDir : public IoRegistrator<MkDir> {
public:
    MkDir(int dfd, const char *path, mode_t mode)
        : IoRegistrator{io_uring_prep_mkdirat, dfd, path, mode} {}

    MkDir(const char *path, mode_t mode)
        : MkDir{AT_FDCWD, path, mode} {}

    auto await_resume() noexcept -> std::expected<void, std::error_code> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return {};
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto mkdir(const char *path, mode_t mode) {
    return detail::MkDir{path, mode};
}

[[REMEMBER_CO_AWAIT]]
static inline auto mkdirat(int dfd, const char *path, mode_t mode) {
    return detail::MkDir{dfd, path, mode};
}

}