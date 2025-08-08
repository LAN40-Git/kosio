#include "runtime/task/waker.h"

coruring::runtime::task::waker::Waker::Waker()
    : fd_{::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)} {
    if (fd_ < 0) [[unlikely]] {
        throw std::runtime_error(
            std::format("call eventfd failed, error: {} - {}", fd_, strerror(errno)));
    }
}

coruring::runtime::task::waker::Waker::~Waker() {
    ::close(fd_);
}

coruring::runtime::task::waker::Waker::Waker(Waker &&other) noexcept
    : fd_(other.fd_) {
    other.fd_ = -1;
}

auto coruring::runtime::task::waker::Waker::operator=(Waker &&other) noexcept -> Waker & {
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

void coruring::runtime::task::waker::Waker::wake_up() const {
    static constexpr uint64_t buf{1};
    if (auto ret = ::write(this->fd_, &buf, sizeof(buf)); ret != sizeof(buf)) [[unlikely]] {
        // LOG_ERROR("Waker write failed, error: {}.", strerror(errno));
    }
}

void coruring::runtime::task::waker::Waker::turn_on() {
    if (flag_ != 0) {
        flag_ = 0;
        auto sqe = io::t_ring->get_sqe();
        assert(sqe != nullptr);
        io_uring_prep_read(sqe, fd_, &flag_, sizeof(flag_), 0);
        io_uring_sqe_set_data(sqe, nullptr);
    }
}
