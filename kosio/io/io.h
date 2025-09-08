#pragma once
#include "kosio/async/coroutine/task.h"
#include "kosio/io/awaiter/accept.h"
#include "kosio/io/awaiter/cancel.h"
#include "kosio/io/awaiter/close.h"
#include "kosio/io/awaiter/connect.h"
#include "kosio/io/awaiter/fsetxattr.h"
#include "kosio/io/awaiter/fsync.h"
#include "kosio/io/awaiter/getxattr.h"
#include "kosio/io/awaiter/link.h"
#include "kosio/io/awaiter/mkdir.h"
#include "kosio/io/awaiter/open.h"
#include "kosio/io/awaiter/read.h"
#include "kosio/io/awaiter/recv.h"
#include "kosio/io/awaiter/rename.h"
#include "kosio/io/awaiter/send.h"
#include "kosio/io/awaiter/shutdown.h"
#include "kosio/io/awaiter/socket.h"
#include "kosio/io/awaiter/splice.h"
#include "kosio/io/awaiter/statx.h"
#include "kosio/io/awaiter/unlink.h"
#include "kosio/io/awaiter/write.h"
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
    auto operator=(FD&& other) noexcept -> FD&;

public:
    [[nodiscard]]
    auto fd() const noexcept { return fd_; }
    [[nodiscard]]
    auto is_valid() const noexcept { return fd_ >= 0; }
    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept -> Close;
    [[nodiscard]]
    auto release() noexcept -> int;
    [[nodiscard]]
    auto set_nonblocking(bool status) const noexcept -> std::expected<void, std::error_code>;
    [[nodiscard]]
    auto nonblocking() const noexcept -> std::expected<bool, std::error_code>;

private:
    void do_close() noexcept;

protected:
    int fd_{-1};
}; 
} // namespace kosio::io::detail