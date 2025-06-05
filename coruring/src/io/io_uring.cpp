#include "io/io_uring.h"

#include <iostream>

coruring::io::IoUring& coruring::io::IoUring::instance() {
    static const Config config = Config::load_from_file();
    thread_local IoUring inst{config};
    return inst;
}

std::unordered_set<void*>& coruring::io::IoUring::callback_map() {
    thread_local std::unordered_set<void*> callback_map{};
    return callback_map;
}

auto coruring::io::IoUring::peek_batch(std::span<io_uring_cqe*> cqes) -> std::size_t {
    return io_uring_peek_batch_cqe(&ring_, cqes.data(), cqes.size());
}

auto coruring::io::IoUring::peek_cqe(io_uring_cqe** cqe) -> int {
    return io_uring_peek_cqe(&ring_, cqe);
}

void coruring::io::IoUring::pend_submit() {
    submit_tick_++;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::io::IoUring::pend_submit_batch(std::size_t count) {
    submit_tick_ += count;
    if (submit_tick_ >= submit_interval_) {
        submit();
    }
}

void coruring::io::IoUring::submit() {
    io_uring_submit(&ring_);
    submit_tick_ = 0;
}

void coruring::io::IoUring::try_submit() {
    if (submit_tick_ > 0) {
        submit();
    }
}

bool coruring::io::IoUring::clear() {
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

coruring::io::IoUring::IoUring(const Config& config)
    : submit_interval_{config.submit_interval} {
    if (auto ret = io_uring_queue_init(config.entries, &ring_, 0); ret < 0) [[unlikely]] {
        throw std::runtime_error(std::format("io_uring_queue_init failed: {}", strerror(-ret)));
    }
}
