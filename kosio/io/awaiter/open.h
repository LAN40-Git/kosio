#pragma once
#include "kosio/io/base/registrator.h"

namespace kosio::io {
namespace detail {
class Open : public IoRegistrator<Open> {
public:
    Open(int dfd, const char *path, int flags, mode_t mode)
        : IoRegistrator{io_uring_prep_openat, dfd, path, flags, mode} {}

    Open(const char *path, int flags, mode_t mode)
        : Open{AT_FDCWD, path, flags, mode} {}

    auto await_resume() const noexcept -> Result<int, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return this->cb_.result_;
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};

class Open2 : public IoRegistrator<Open2> {
public:
    Open2(int dfd, const char *path, struct open_how *how)
        : IoRegistrator{io_uring_prep_openat2, dfd, path, how} {}

    Open2(const char *path, struct open_how *how)
        : Open2{AT_FDCWD, path, how} {}

    auto await_resume() noexcept -> std::expected<int, std::error_code> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return this->cb_.result_;
        }
        return std::unexpected{std::error_code(-this->cb_.result_,
                                               std::generic_category())};
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto open(const char *path, int flags, mode_t mode) {
    return detail::Open{path, flags, mode};
}

[[REMEMBER_CO_AWAIT]]
static inline auto open2(const char *path, struct open_how *how) {
    return detail::Open2{path, how};
}

[[REMEMBER_CO_AWAIT]]
static inline auto openat(int dfd, const char *path, int flags, mode_t mode) {
    return detail::Open{dfd, path, flags, mode};
}

[[REMEMBER_CO_AWAIT]]
static inline auto openat2(int dfd, const char *path, struct open_how *how) {
    return detail::Open2{dfd, path, how};
}
} // namespace kosio::io