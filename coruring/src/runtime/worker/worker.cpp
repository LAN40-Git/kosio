#include <runtime/io/io_uring.h>
#include "runtime/worker/worker.h"
#include "io/base/callback.h"
#include "runtime/timer/entry.h"
#include "scheduler/scheduler.h"

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
    std::cout << "stopping" << std::endl;
}

void coruring::runtime::detail::Worker::event_loop() {
    std::array<io_uring_cqe*, Config::PEEK_BATCH_SIZE> cqes{};
    auto& workers = scheduler_.workers();
    while (is_running()) {
        // 1. 处理IO事件
        std::size_t count = local_queue_.try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        for (std::size_t i = 0; i < count; ++i) {
            if (auto handle = io_buf_[i]) {
                handle.resume();
                if (handle.done()) {
                    scheduler_.erase(handle);
                }
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
                local_queue_.enqueue(cb->handle_);
                cb->result_ = cqes[i]->res;
            }
        }
        if (count > 0) {
            IoUring::instance().consume(count);
        } else {
            if (local_queue_.size_approx() == 0) {
                IoUring::instance().wait(0, 1000000);
            }
        }

        // 3. 推进时间轮
        Timer::instance().tick();

        // 4. 窃取任务
        // 窃取全局队列
        count = scheduler_.global_queue().try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        local_queue_.enqueue_bulk(io_buf_.begin(), count);

        // 5. 立即提交请求
        IoUring::instance().submit();
    }
}