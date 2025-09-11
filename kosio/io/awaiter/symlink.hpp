#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class SymLink : IoRegistrator<SymLink> {
private:
    using Super = IoRegistrator<SymLink>;

public:
    SymLink(const char *target, int newdirfd, const char *linkpath)
        : Super{io_uring_prep_symlinkat, target, newdirfd, linkpath} {}

    SymLink(const char *target, const char *linkpath)
        : Super{io_uring_prep_symlink, target, linkpath} {}

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
static inline auto symlink(const char *target, const char *linkpath) {
    return detail::SymLink{target, linkpath};
}

[[REMEMBER_CO_AWAIT]]
static inline auto symlinkat(const char *target, int newdirfd, const char *linkpath) {
    return detail::SymLink{target, newdirfd, linkpath};
}
} // namespace kosio::io