#include "runtime/io/io_uring.h"
#include <iostream>

coruring::runtime::detail::IoUring& coruring::runtime::detail::IoUring::instance() {
    static const Config config = Config::load_from_file();
    thread_local IoUring inst{config};
    return inst;
}

auto coruring::runtime::detail::IoUring::peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
    return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
}

auto coruring::runtime::detail::IoUring::peek_cqe(io_uring_cqe** cqe) -> int {
    return io_uring_peek_cqe(&ring_, cqe);
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
    : submit_interval_{config.submit_interval} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
}
