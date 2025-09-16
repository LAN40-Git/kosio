#pragma once
#include "kosio/common/error.hpp"
#include "kosio/async/coroutine/task.hpp"
#include "kosio/io/awaiter/accept.hpp"
#include "kosio/io/awaiter/cancel.hpp"
#include "kosio/io/awaiter/close.hpp"
#include "kosio/io/awaiter/connect.hpp"
#include "kosio/io/awaiter/fsetxattr.hpp"
#include "kosio/io/awaiter/fsync.hpp"
#include "kosio/io/awaiter/getxattr.hpp"
#include "kosio/io/awaiter/link.hpp"
#include "kosio/io/awaiter/mkdir.hpp"
#include "kosio/io/awaiter/open.hpp"
#include "kosio/io/awaiter/read.hpp"
#include "kosio/io/awaiter/recv.hpp"
#include "kosio/io/awaiter/rename.hpp"
#include "kosio/io/awaiter/send.hpp"
#include "kosio/io/awaiter/sendmsg.hpp"
#include "kosio/io/awaiter/sendto.hpp"
#include "kosio/io/awaiter/setxattr.hpp"
#include "kosio/io/awaiter/shutdown.hpp"
#include "kosio/io/awaiter/socket.hpp"
#include "kosio/io/awaiter/splice.hpp"
#include "kosio/io/awaiter/statx.hpp"
#include "kosio/io/awaiter/symlink.hpp"
#include "kosio/io/awaiter/unlink.hpp"
#include "kosio/io/awaiter/write.hpp"
#include <string>
#include <fcntl.h>
#include <expected>

namespace kosio::io::detail {
class FD {
public:
    FD(const FD&) = delete;
    auto operator=(const FD&) -> FD& = delete;

protected:
    explicit FD(int fd = -1) noexcept : fd_(fd) {}
    ~FD() {
        if (fd_ >= 0) {
            do_close();
        }
    }
    FD(FD&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    auto operator=(FD&& other) noexcept -> FD& {
        if (fd_ >= 0) {
            do_close();
        }
        fd_ = other.fd_;
        other.fd_ = -1;
        return *this;
    }

public:
    [[nodiscard]]
    auto fd() const noexcept { return fd_; }

    [[nodiscard]]
    auto is_valid() const noexcept { return fd_ >= 0; }

    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept -> Close {
        auto fd = fd_;
        fd_ = -1;
        return Close{fd};
    }

    void sync_close() noexcept {
        if (!is_valid()) {
            return;
        }
        for (auto i = 0; i < 3; ++i) {
            auto ret = ::close(fd_);
            if (ret == 0) [[likely]] {
                break;
            }
            // Failed to close
        }
        fd_ = -1;
    }

    [[nodiscard]]
    auto release() noexcept -> int {
        int released_fd = fd_;
        fd_ = -1;
        return released_fd;
    }

    [[nodiscard]]
    auto set_nonblocking(bool status) const noexcept -> Result<void> {
        auto flags = ::fcntl(fd_, F_GETFL, 0);
        if (status) {
            flags |= O_NONBLOCK;
        } else {
            flags &= ~O_NONBLOCK;
        }
        if (::fcntl(fd_, F_SETFL, flags) == -1) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return {};
    }

    [[nodiscard]]
    auto nonblocking() const noexcept -> Result<bool> {
        auto flags = ::fcntl(fd_, F_GETFL, 0);
        if (flags == -1) [[unlikely]] {
            return std::unexpected{make_error(errno)};
        }
        return {true};
    }

private:
    void do_close() noexcept {
        auto sqe = runtime::io::t_ring->get_sqe();
        if (sqe) [[likely]] {
            // async close
            io_uring_prep_close(sqe, fd_);
            io_uring_sqe_set_data(sqe, nullptr);
            runtime::io::t_ring->pend_submit();
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

protected:
    int fd_{-1};
}; 
} // namespace kosio::io::detail