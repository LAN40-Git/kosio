#include "runtime/io/io_uring.h"
#include <iostream>

#include "common/debug.h"

coruring::runtime::io::IoUring::IoUring(const detail::Config& config)
    : submit_interval_{static_cast<uint32_t>(config.submit_interval)} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
    assert(t_ring == nullptr);
    t_ring = this;
}

coruring::runtime::io::IoUring::~IoUring() {
    t_ring = nullptr;
    io_uring_queue_exit(&ring_);
}

auto coruring::runtime::io::IoUring::peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
    return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
}

auto coruring::runtime::io::IoUring::peek_cqe(io_uring_cqe** cqe) -> int {
    return io_uring_peek_cqe(&ring_, cqe);
}

void coruring::runtime::io::IoUring::wait(std::optional<time_t> timeout_ms) {
    // TODO: 记录错误
    io_uring_cqe *cqe{nullptr};
    if (timeout_ms) {
        __kernel_timespec ts{
            .tv_sec = timeout_ms.value() /1000,
            .tv_nsec = timeout_ms.value() % 1000 * 1000'000
        };
        io_uring_wait_cqe_timeout(&ring_, &cqe, &ts);
    } else {
        io_uring_wait_cqe(&ring_, &cqe);
    }
}

void coruring::runtime::io::IoUring::pend_submit() {
    submit_tick_++;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::runtime::io::IoUring::pend_submit_batch(std::size_t count) {
    submit_tick_ += count;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::runtime::io::IoUring::submit() {
    submit_tick_ = 0;
    if (auto ret = io_uring_submit(&ring_); ret < 0) [[unlikely]] {
        std::cerr << "io_uring_submit failed: " << ret << std::endl;
    }
}
