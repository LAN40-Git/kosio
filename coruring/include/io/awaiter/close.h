#pragma once
#include "io/base/registrator.h"

namespace coruring::io {
namespace detail {
class Close : public IoRegistrator<Close> {
public:
    Close(int fd)
        : IoRegistrator{io_uring_prep_close, fd} {}

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
static inline auto close(int fd) {
    return detail::Close {fd};
}
} // namespace coruring::io
