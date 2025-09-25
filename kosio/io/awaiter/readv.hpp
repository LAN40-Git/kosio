#pragma once
#include "kosio/io/base/registrator.hpp"

namespace kosio::io {
namespace detail {
class ReadV : public IoRegistrator<ReadV> {
private:
    using Super = IoRegistrator<ReadV>;

public:
    ReadV(int fd, const struct iovec *iovecs, unsigned nr_vecs, __u64 offset, int flags)
        : Super{io_uring_prep_readv2, fd, iovecs, nr_vecs, offset, flags} {}

    auto await_resume() const noexcept -> Result<std::size_t> {
        if (this->cb_.result_ >= 0) [[likely]] {
            return static_cast<std::size_t>(this->cb_.result_);
        } else {
            return std::unexpected{make_error(-this->cb_.result_)};
        }
    }
};
} // namespace detail
[[REMEMBER_CO_AWAIT]]
static inline auto
readv(int fd, const struct iovec *iovecs, unsigned nr_vecs, __u64 offset, int flags = 0) {
    return detail::ReadV{fd, iovecs, nr_vecs, offset, flags};
}
} // namespace kosio::io
