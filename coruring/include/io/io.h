#pragma once
#include <expected>

#include "async/coroutine/task.h"
#include "io/awaiter/accept.h"
#include "io/awaiter/cancel.h"
#include "io/awaiter/close.h"
#include "io/awaiter/connect.h"
#include "io/awaiter/fsync.h"
#include "io/awaiter/mkdir.h"
#include "io/awaiter/open.h"
#include "io/awaiter/read.h"
#include "io/awaiter/recv.h"
#include "io/awaiter/rename.h"
#include "io/awaiter/send.h"
#include "io/awaiter/shutdown.h"
#include "io/awaiter/write.h"
#include <string>
#include <fcntl.h>

namespace coruring::io::detail
{
class FD
{
public:
    FD(const FD&) = delete;
    auto operator=(const FD&) -> FD& = delete;

protected:
    explicit FD(int fd = -1) noexcept : fd_(fd) {}
    ~FD() { do_close(); }
    FD(FD&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    auto operator=(FD&& other) noexcept -> FD&;

public:
    [[nodiscard]]
    auto fd() const noexcept { return fd_; }
    [[nodiscard]]
    auto is_valid() const noexcept { return fd_ >= 0; }
    [[REMEMBER_CO_AWAIT]]
    auto close() noexcept -> detail::Close;
    [[nodiscard]]
    auto release() noexcept -> int;
    [[nodiscard]]
    auto set_nonblocking(bool status) const noexcept -> std::error_code;
    [[nodiscard]]
    auto nonblocking() const noexcept -> std::expected<bool, std::error_code>;

private:
    void do_close() noexcept;

protected:
    int fd_{-1};
}; 
} // namespace coruring::utils