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
        active_tasks_ = IoUring::data_set().size();
        // 1. 处理IO事件
        std::size_t count = local_queue_.try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        for (std::size_t i = 0; i < count; ++i) {
            if (io_buf_[i]) {
                io_buf_[i].resume();
            }
        }

        // 2. 收割完成队列
        count = IoUring::instance().peek_batch(cqes_);
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
                constexpr long long NS_PER_MS = 1000000;
                IoUring::instance().wait(0, 2*NS_PER_MS);
            }
        }

        // 3. 推进时间轮
        timer_.tick();

        // 4. 尝试窃取任务（削峰窃取）
        std::size_t size = tasks();
        // 首先窃取全局队列
        count = scheduler_.global_queue().try_dequeue_bulk(io_buf_.begin(), io_buf_.size());
        local_queue_.enqueue_bulk(io_buf_.begin(), count);
        size += count;
        for (auto& worker : scheduler_.workers()) {
            if (worker.get() == this) {
                continue;
            }
            if (worker->is_running()) [[likely]] {
                auto& local_queue = worker->local_queue();
                // 计算平均值
                std::size_t average = (size + worker->tasks()) / 2;
                if (average > size + static_cast<std::size_t>(Config::STEAL_FACTOR*size)) {
                    count = local_queue.try_dequeue_bulk(io_buf_.begin(),
                                            std::min(io_buf_.size(), average - size));
                    local_queue_.enqueue_bulk(io_buf_.begin(), count);
                    break;
                }
            }
        }

        // 5. 尝试立即提交请求
        IoUring::instance().try_submit();
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
}
