#include <runtime/io/io_uring.h>
#include "runtime/worker/worker.h"
#include "io/base/callback.h"
#include "runtime/timer/entry.h"

void coruring::runtime::detail::Worker::run() {
    std::lock_guard lock(mutex_);
    if (is_running_) {
        return;
    }
    is_running_.store(true, std::memory_order_release);
    thread_ = std::thread([&]() {
        event_loop();
    });
}

void coruring::runtime::detail::Worker::stop() {
    std::lock_guard lock(mutex_);
    if (!is_running_) {
        return;
    }
    is_running_.store(false, std::memory_order_release);
    if (thread_.joinable()) {
        thread_.join();
    }
}

void coruring::runtime::detail::Worker::event_loop() {
    std::array<std::coroutine_handle<>, Config::IO_INTERVAL> io_buf;
    std::array<io_uring_cqe *, Config::IO_INTERVAL> cqes;
    while (is_running_.load(std::memory_order_relaxed)) {
        // 1. 处理IO事件
        std::size_t count = local_queue_.try_dequeue_bulk(std::make_move_iterator(io_buf.begin()), io_buf.size());
        for (std::size_t i = 0; i < count; ++i) {
            if (io_buf[i]) {
                io_buf[i].resume();
            }
        }

        // 2. 收割完成队列
        count = IoUring::instance().peek_batch(cqes);
        for (std::size_t i = 0; i < count; ++i) {
            auto cb = reinterpret_cast<io::detail::Callback *>(cqes[i]->user_data);
            if (cb != nullptr) [[likely]] {
                if (cb->entry_ != nullptr) {
                    cb->entry_->data_ = nullptr;
                }
                cb->result_ = cqes[i]->res;
                local_queue_.enqueue(std::move(cb->handle_));
            }
        }
        if (count > 0) {
            IoUring::instance().consume(count);
        } else {
            constexpr long long NS_PER_US = 1000;
            IoUring::instance().wait(0, 500*NS_PER_US);
        }

        // 3. 推进时间轮（仅在时间差>=1ms时真实推进）
        timer_.tick();

        // 4. 尝试窃取任务

    }
}
