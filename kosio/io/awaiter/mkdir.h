#pragma once
#include "kosio/io/base/registrator.h"

namespace kosio::io {
namespace detail {
class MkDir : public IoRegistrator<MkDir> {
public:
    MkDir(int dfd, const char *path, mode_t mode)
        : IoRegistrator{io_uring_prep_mkdirat, dfd, path, mode} {}

    MkDir(const char *path, mode_t mode)
        : MkDir{AT_FDCWD, path, mode} {}

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
static inline auto mkdir(const char *path, mode_t mode) {
    return detail::MkDir{path, mode};
}

[[REMEMBER_CO_AWAIT]]
static inline auto mkdirat(int dfd, const char *path, mode_t mode) {
    return detail::MkDir{dfd, path, mode};
}

}