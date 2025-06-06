#include "../../include/io/io.h"

auto coruring::util::FileDescriptor::operator=(FileDescriptor&& other) noexcept -> FileDescriptor& {
    if (this != &other) {
        this->close();
        this->fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void coruring::util::FileDescriptor::close() noexcept {
    if (fd_ < 0) {
        return;
    }
    for (int i = 0; i < 3; ++i) {  // 最多重试 3 次
        if (::close(fd_) == 0) [[likely]] {
            fd_ = -1;
            return;
        }
    }
    fd_ = -1;
}
