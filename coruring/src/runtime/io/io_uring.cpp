#include "../../../include/runtime/io/io_uring.h"

#include <iostream>

coruring::io::detail::IoUring& coruring::io::detail::IoUring::instance() {
    static const runtime::detail::Config config = runtime::detail::Config::load_from_file();
    thread_local IoUring inst{config};
    return inst;
}

auto coruring::io::detail::IoUring::peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
    return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
}

auto coruring::io::detail::IoUring::peek_cqe(io_uring_cqe** cqe) -> int {
    return io_uring_peek_cqe(&ring_, cqe);
}

void coruring::io::detail::IoUring::pend_submit() {
    submit_tick_++;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::io::detail::IoUring::pend_submit_batch(std::size_t count) {
    submit_tick_ += count;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::io::detail::IoUring::submit() {
    io_uring_submit(&ring_);
    submit_tick_ = 0;
}

void coruring::io::detail::IoUring::try_submit() {
    if (submit_tick_ > 0) {
        submit();
    }
}

coruring::io::detail::IoUring::IoUring(const runtime::detail::Config& config)
    : submit_interval_{config.submit_interval} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
}
