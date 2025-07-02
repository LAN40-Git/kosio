#include "runtime/io/io_uring.h"
#include "runtime/worker/worker.h"
#include "io/base/callback.h"
#include "runtime/timer/entry.h"
#include "runtime/scheduler.h"
#include "common/util/random.h"

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
    std::array<io_uring_cqe*, Config::PEEK_BATCH_SIZE> cqes{};
    std::array<std::coroutine_handle<>, Config::IO_BATCH_SIZE> io_buf;
    auto& workers = scheduler_.workers();
    int32_t worker_nums = scheduler_.worker_nums();
    auto& handles = scheduler_.handles();
    long long wait_ms = 1;
    while (is_running()) {
        // 1. 处理IO事件
        std::size_t count = local_queue_.try_dequeue_bulk(io_buf.begin(), io_buf.size());
        for (std::size_t i = 0; i < count; ++i) {
            io_buf[i].resume();
            if (io_buf[i].done() && handles.erase(io_buf[i])) {
                // 销毁管理的协程
                io_buf[i].destroy();
                remove_tasks(1);
            }
        }

        // 2. 立即提交请求
        IoUring::instance().submit();

        // 3. 收割完成队列
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
            wait_ms = 1;
        } else {
            if (local_queue_.size_approx() == 0 && scheduler_.global_queue().size_approx() == 0) {
                constexpr long long NS_PER_MS = 1000000;
                if (Timer::instance().is_idle()) {
                    wait_ms = std::min(wait_ms*2, 100LL);
                } else {
                    wait_ms = 1;
                }
                IoUring::instance().wait(0, wait_ms*NS_PER_MS);
            }
        }

        // 4. 推进分层时间轮
        Timer::instance().tick();

        // 5. 窃取任务
        // 从全局队列窃取
        count = scheduler_.global_queue().try_dequeue_bulk(io_buf.begin(),io_buf.size());
        local_queue_.enqueue_bulk(io_buf.begin(), count);
        add_tasks(count);
        // 从其它线程窃取（多线程时）
        if (worker_nums <= 1) {
            continue;
        }
        auto tasks = local_tasks();
        auto average_tasks = handles.size()/worker_nums; // 计算平均任务
        if (tasks * Config::STEAL_FACTOR < average_tasks) {
            auto worker_idx = util::FastRand::instance().rand_range(0, worker_nums-1);
            auto& worker = workers[worker_idx];
            if (worker.get() == this) {
                continue;
            }
            std::size_t steal_threshold = average_tasks * Config::STEAL_FACTOR; // 窃取阈值（窃取任务数大于此阈值的线程）
            auto peer_tasks = worker->local_tasks();
            if (peer_tasks > steal_threshold) {
                auto& peer_queue = worker->local_queue();
                count = peer_queue.try_dequeue_bulk(io_buf.begin(), std::min(io_buf.size(), average_tasks - tasks));
                worker->remove_tasks(count);
                local_queue_.enqueue_bulk(io_buf.begin(), count);
                add_tasks(count);
            }
        }
    }
}