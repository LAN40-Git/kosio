#include "io/io_uring.h"

#include <iostream>

coruring::io::detail::IoUring& coruring::io::detail::IoUring::instance() {
    static const runtime::Config config = runtime::Config::load_from_file();
    thread_local IoUring inst{config};
    return inst;
}

std::unordered_set<void*>& coruring::io::detail::IoUring::callback_map() {
    thread_local std::unordered_set<void*> callback_map{};
    return callback_map;
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

bool coruring::io::detail::IoUring::clear() {
    auto &callback_map = IoUring::callback_map();
    std::size_t count = callback_map.size();
    if (count == 0) {
        return true;
    }
    for (auto user_data : callback_map) {
        io_uring_sqe* sqe = get_sqe();
        if (!sqe) [[unlikely]] {
            return false;
        }
        io_uring_prep_cancel(sqe, user_data, 0);
        io_uring_sqe_set_data(sqe, nullptr);
    }
    pend_submit_batch(count);
    return true;
}

coruring::io::detail::IoUring::IoUring(const runtime::Config& config)
    : submit_interval_{config.submit_interval} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
}
