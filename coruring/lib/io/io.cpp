#include "io/io.h"

auto coruring::io::detail::FD::operator=(FD&& other) noexcept -> FD& {
    if (fd_ >= 0) {
        do_close();
    }
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

auto coruring::io::detail::FD::close() noexcept -> Close {
    auto fd = fd_;
    fd_ = -1;
    return Close{fd};
}

auto coruring::io::detail::FD::release() noexcept -> int {
    int released_fd = fd_;
    fd_ = -1;
    return released_fd;
}

auto coruring::io::detail::FD::set_nonblocking(bool status) const noexcept -> std::expected<void, std::error_code> {
    auto flags = ::fcntl(fd_, F_GETFL, 0);
    if (status) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (::fcntl(fd_, F_SETFL, flags) == -1) [[unlikely]] {
        return std::unexpected{std::error_code{errno, std::generic_category()}};
    }
    return {};
}

auto coruring::io::detail::FD::nonblocking() const noexcept -> std::expected<bool, std::error_code> {
    auto flags = ::fcntl(fd_, F_GETFL, 0);
    if (flags == -1) [[unlikely]] {
        return std::unexpected{std::error_code(errno, std::generic_category())};
    }
    return {true};
}

void coruring::io::detail::FD::do_close() noexcept {
    auto sqe = runtime::detail::IoUring::instance().get_sqe();
    if (sqe) [[likely]] {
        // async close
        io_uring_prep_close(sqe, fd_);
        io_uring_sqe_set_data(sqe, nullptr);
        runtime::detail::IoUring::instance().pend_submit();
    } else {
        // sync close
        for (auto i = 0; i < 3; ++i) {
            auto ret = ::close(fd_);
            if (ret == 0) [[likely]] {
                break;
            }
            // Failed to close
        }
    }
    fd_ = -1;
}
