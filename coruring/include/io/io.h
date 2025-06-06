#pragma once
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
#include "io/awaiter/timeout.h"
#include "io/awaiter/timeout_recv.h"
#include "io/awaiter/write.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace coruring::util
{
class FD
{
public:
    explicit FD(int fd = -1) noexcept : fd_(fd) {}
    ~FD() {
        close();
    }

    FD(const FD&) = delete;
    auto operator=(const FD&) -> FD& = delete;
    FD(FD&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    auto operator=(FD&& other) noexcept -> FD&;

public:
    [[nodiscard]]
    auto fd() const noexcept { return fd_; }
    [[nodiscard]]
    auto is_valid() const noexcept { return fd_ >= 0; }
    void close() noexcept;
    [[nodiscard]]
    auto release() noexcept -> int {
        int released_fd = fd_;
        fd_ = -1;
        return released_fd;
    }

private:


protected:
    int fd_{-1};
}; 
} // namespace coruring::utils