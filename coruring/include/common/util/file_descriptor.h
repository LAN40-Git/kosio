#pragma once
#include "coruring/include/common/utils/debug.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace coruring::util
{
class FileDescriptor
{
public:
    explicit FileDescriptor(int fd = -1) noexcept : fd_(fd) {}
    ~FileDescriptor() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    FileDescriptor(const FileDescriptor&) = delete;
    auto operator=(const FileDescriptor&) -> FileDescriptor& = delete;

    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor& {
        if (this != &other) {
            this->close();
            this->fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

public:

    auto fd() const noexcept { return fd_; }

    auto is_valid() const noexcept { return fd_ >= 0; }

    void close() noexcept {
        if (fd_ < 0) {
            return;
        }
        for (int i = 0; i < 3; ++i) {  // 最多重试 3 次
            if (::close(fd_) == 0) {
                fd_ = -1;
                return;
            }
        }
        fd_ = -1;
    }

    int release() noexcept {
        int released_fd = fd_;
        fd_ = -1;
        return released_fd;
    }

    static FileDescriptor create(int domain, int type, int protocol = 0) {
        int fd = ::socket(domain, type, protocol);
        return FileDescriptor(fd);
    }
    
    static FileDescriptor create(const std::string &filename) {
        int fd = ::open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0777);
        return FileDescriptor(fd);
    }

private:
    int fd_{-1};
}; 
} // namespace coruring::utils