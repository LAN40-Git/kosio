#pragma once
#include "kosio/include/async/coroutine/task.h"
#include "kosio/include/io/awaiter/accept.h"
#include "kosio/include/io/awaiter/cancel.h"
#include "kosio/include/io/awaiter/close.h"
#include "kosio/include/io/awaiter/connect.h"
#include "kosio/include/io/awaiter/fsetxattr.h"
#include "kosio/include/io/awaiter/fsync.h"
#include "kosio/include/io/awaiter/getxattr.h"
#include "kosio/include/io/awaiter/link.h"
#include "kosio/include/io/awaiter/mkdir.h"
#include "kosio/include/io/awaiter/open.h"
#include "kosio/include/io/awaiter/read.h"
#include "kosio/include/io/awaiter/recv.h"
#include "kosio/include/io/awaiter/rename.h"
#include "kosio/include/io/awaiter/send.h"
#include "kosio/include/io/awaiter/shutdown.h"
#include "kosio/include/io/awaiter/socket.h"
#include "kosio/include/io/awaiter/splice.h"
#include "kosio/include/io/awaiter/statx.h"
#include "kosio/include/io/awaiter/unlink.h"
#include "kosio/include/io/awaiter/write.h"
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