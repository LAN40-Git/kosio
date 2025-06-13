#include "runtime/io/io_uring.h"
#include <iostream>

coruring::runtime::detail::IoUring& coruring::runtime::detail::IoUring::instance() {
    thread_local IoUring inst{Config::load()};
    return inst;
}

auto coruring::runtime::detail::IoUring::peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
    return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
}

auto coruring::runtime::detail::IoUring::peek_cqe(io_uring_cqe** cqe) -> int {
    return io_uring_peek_cqe(&ring_, cqe);
}

void coruring::runtime::detail::IoUring::wait() {
    io_uring_cqe *cqe{nullptr};
    io_uring_wait_cqe(&ring_, &cqe);
}

void coruring::runtime::detail::IoUring::wait(long long tv_sec, long long tv_nsec) {
    io_uring_cqe *cqe{nullptr};
    struct __kernel_timespec ts{.tv_sec = tv_sec, .tv_nsec = tv_nsec};
    io_uring_wait_cqe_timeout(&ring_, &cqe, &ts);
}

void coruring::runtime::detail::IoUring::pend_submit() {
    submit_tick_++;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::runtime::detail::IoUring::pend_submit_batch(std::size_t count) {
    submit_tick_ += count;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::runtime::detail::IoUring::submit() {
    io_uring_submit(&ring_);
    submit_tick_ = 0;
}

void coruring::runtime::detail::IoUring::try_submit() {
    if (submit_tick_ > 0) {
        submit();
    }
}

coruring::runtime::detail::IoUring::IoUring(const Config& config)
    : submit_interval_{static_cast<uint32_t>(config.submit_interval)} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
}
