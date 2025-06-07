#pragma once
#include "io/base/registrator.h"

namespace coruring::io
{
namespace detail
{
    class Splice : public IoRegistrator<Splice> {
    public:
        Splice(int          fd_in,
               int64_t      off_in,
               int          fd_out,
               int64_t      off_out,
               unsigned int nbytes,
               unsigned int splice_flags)
            : IoRegistrator{io_uring_prep_splice, fd_in, off_in, fd_out, off_out, nbytes, splice_flags} {}

        auto await_resume() const noexcept -> std::expected<std::size_t, std::error_code> {
            if (this->cb_.result_ >= 0) [[likely]] {
                return static_cast<std::size_t>(this->cb_.result_);
            }
            return ::std::unexpected{std::error_code(-this->cb_.result_,
            std::generic_category())};
        }
    };
}

[[REMEMBER_CO_AWAIT]]
static inline auto splice(int          fd_in,
                          int64_t      off_in,
                          int          fd_out,
                          int64_t      off_out,
                          unsigned int nbytes,
                          unsigned int splice_flags) {
    return detail::Splice{fd_in, off_in, fd_out, off_out, nbytes, splice_flags};
}
} // namespace coruring::io