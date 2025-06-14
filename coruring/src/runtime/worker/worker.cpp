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
}

void coruring::runtime::detail::Worker::event_loop() {
    while (is_running()) {
        // 1. 处理IO事件
        while (local_queue_.size_approx() != 0) {
            std::size_t count = local_queue_.try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
            for (std::size_t i = 0; i < count; ++i) {
                if (io_buf_[i]) {
                    io_buf_[i].resume();
                }
            }
        }

        // 2. 收割完成队列
        std::size_t count = IoUring::instance().peek_batch(cqes_);
        for (std::size_t i = 0; i < count; ++i) {
            auto cb = reinterpret_cast<io::detail::Callback *>(cqes_[i]->user_data);
            if (cb != nullptr) [[likely]] {
                if (cb->entry_ != nullptr) {
                    cb->entry_->data_ = nullptr;
                }
                local_queue_.enqueue(cb->handle_);
                cb->result_ = cqes_[i]->res;
                IoUring::data_set().erase(cb);
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
        count = scheduler_.global_queue().try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        local_queue_.enqueue_bulk(io_buf_.begin(), count);


        // 5. 立即提交请求
        IoUring::instance().submit();
    }
    // 释放资源
    clear();
}

void coruring::runtime::detail::Worker::clear() noexcept {
    // 1. 清理所有请求
    IoUring::instance().cancle_all_request();
    while (!IoUring::data_set().empty()) {
        std::size_t count = IoUring::instance().peek_batch(cqes_);
        for (std::size_t i = 0; i < count; ++i) {
            auto cb = reinterpret_cast<io::detail::Callback *>(cqes_[i]->user_data);
            if (cb != nullptr) [[likely]] {
                if (cb->entry_ != nullptr) {
                    cb->entry_->data_ = nullptr;
                }
                cb->result_ = cqes_[i]->res;
                if (cb->handle_) {
                    cb->handle_.destroy();
                }
                IoUring::data_set().erase(cb);
            }
        }
    }
    // 2. 清除本地队列
    while (local_queue_.size_approx() != 0) {
        std::size_t count = local_queue_.try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        for (std::size_t i = 0; i < count; ++i) {
            if (io_buf_[i]) {
                io_buf_[i].destroy();
            }
        }
    }
    // 3. 清理定时器
    Timer::instance().clear();
}