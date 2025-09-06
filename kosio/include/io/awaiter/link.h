#pragma once
#include "kosio/include/io/base/registrator.h"

namespace kosio::io {
namespace detail {
class Link : public IoRegistrator<Link> {
public:
    Link(int olddfd, const char *oldpath, int newdfd, const char *newpath, int flags)
        : IoRegistrator(io_uring_prep_linkat, olddfd, oldpath, newdfd, newpath, flags) {}

    Link(const char *oldpath, const char *newpath, int flags)
        : IoRegistrator{io_uring_prep_link, oldpath, newpath, flags} {}

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
static inline auto link(const char *oldpath, const char *newpath, int flags) {
    return detail::Link{oldpath, newpath, flags};
}

[[REMEMBER_CO_AWAIT]]
static inline auto
linkat(int olddfd, const char *oldpath, int newdfd, const char *newpath, int flags) {
    return detail::Link{olddfd, oldpath, newdfd, newpath, flags};
}
} // namespace kosio::io