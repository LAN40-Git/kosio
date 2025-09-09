#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class Splice : public IoRegistrator<Splice> {
public:
    Splice(int          fd_in,
           int64_t      off_in,
           int          fd_out,
           int64_t      off_out,
           unsigned int nbytes,
           unsigned int splice_flags)
        : IoRegistrator{io_uring_prep_splice, fd_in, off_in, fd_out, off_out, nbytes, splice_flags} {}

    auto await_resume() const noexcept -> Result<std::size_t, IoError> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return std::unexpected{make_error<IoError>(-this->cb_.result_)};
        }
    }
};
} // namespace detail

[[REMEMBER_CO_AWAIT]]
static inline auto splice(int          fd_in,
                          int64_t      off_in,
                          int          fd_out,
                          int64_t      off_out,
                          unsigned int nbytes,
                          unsigned int splice_flags) {
    return detail::Splice{fd_in, off_in, fd_out, off_out, nbytes, splice_flags};
}
} // namespace kosio::io