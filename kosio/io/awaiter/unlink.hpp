#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class Unlink : public IoRegistrator<Unlink> {
public:
    Unlink(int dfd, const char *path, int flags)
        : IoRegistrator{io_uring_prep_unlinkat, dfd, path, flags} {}

    Unlink(const char *path, int flags)
        : IoRegistrator{io_uring_prep_unlink, path, flags} {}

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
static inline auto unlinkat(int dfd, const char *path, int flags) {
    return detail::Unlink(dfd, path, flags);
}

[[REMEMBER_CO_AWAIT]]
static inline auto unlink(const char *path, int flags) {
    return detail::Unlink{path, flags};
}
} // namespace kosio::io