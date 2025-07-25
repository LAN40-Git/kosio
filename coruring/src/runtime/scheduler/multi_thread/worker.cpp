#include "runtime/io/io_uring.h"
#include "runtime/scheduler/multi_thread/worker.h"
#include "io/base/callback.h"
#include "runtime/timer/entry.h"
#include "runtime/scheduler/scheduler.h"
#include "common/util/random.h"

void coruring::runtime::multi_thread::detail::Worker::run() {
    std::lock_guard lock(mutex_);
    if (is_running_) {
        return;
    }
    is_running_.store(true, std::memory_order_release);
    thread_ = std::thread([&]() {
        event_loop();
    });
}

void coruring::runtime::multi_thread::detail::Worker::stop() {
    std::lock_guard lock(mutex_);
    if (!is_running_) {
        return;
    }
    is_running_.store(false, std::memory_order_release);
    if (thread_.joinable()) {
        thread_.join();
    }
}

void coruring::runtime::multi_thread::detail::Worker::event_loop() {
    std::array<io_uring_cqe*, runtime::detail::Config::PEEK_BATCH_SIZE> cqes{};
    std::array<std::coroutine_handle<>, runtime::detail::Config::IO_BATCH_SIZE> event_buf;
    auto& workers = scheduler_.workers();
    auto& handles = scheduler_.handles();
    long long wait_ms = 1;
    while (is_running()) {
        // 1. 处理IO事件
        std::size_t count = local_queue_.try_dequeue_bulk(event_buf.begin(), event_buf.size());
        for (std::size_t i = 0; i < count; ++i) {
            event_buf[i].resume();
            if (event_buf[i].done() && handles.erase(event_buf[i])) {
                // 销毁管理的协程
                event_buf[i].destroy();
                remove_tasks(1);
            }
        }

        // 2. 立即提交请求
        runtime::detail::IoUring::instance().submit();

        // 3. 收割完成队列
        count = runtime::detail::IoUring::instance().peek_batch(cqes);
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
            runtime::detail::IoUring::instance().consume(count);
            wait_ms = 1;
        } else {
            if (local_queue_.size_approx() == 0 && scheduler_.global_queue().size_approx() == 0) {
                constexpr long long NS_PER_MS = 1000000;
                if (Timer::instance().is_idle()) {
                    wait_ms = std::min(wait_ms*2, 100LL);
                } else {
                    wait_ms = 1;
                }
                runtime::detail::IoUring::instance().wait(0, wait_ms*NS_PER_MS);
            }
        }

        // 4. 推进分层时间轮
        Timer::instance().tick();

        // 5. 窃取任务
        // 从全局队列窃取
        count = scheduler_.global_queue().try_dequeue_bulk(event_buf.begin(),event_buf.size());
        local_queue_.enqueue_bulk(event_buf.begin(), count);
        add_tasks(count);
        // 从其它线程窃取（多线程时）
        auto worker_nums = scheduler_.workers().size();
        if (worker_nums <= 1) {
            continue;
        }
        auto tasks = local_tasks();
        auto average_tasks = handles.size()/worker_nums; // 计算平均任务
        if (tasks * runtime::detail::Config::STEAL_FACTOR < average_tasks) {
            auto worker_idx = util::FastRand::instance().rand_range(0, worker_nums-1);
            auto& worker = workers[worker_idx];
            if (worker.get() == this) {
                continue;
            }
            std::size_t steal_threshold = average_tasks * runtime::detail::Config::STEAL_FACTOR; // 窃取阈值（窃取任务数大于此阈值的线程）
            auto peer_tasks = worker->local_tasks();
            if (peer_tasks > steal_threshold) {
                auto& peer_queue = worker->local_queue();
                count = peer_queue.try_dequeue_bulk(event_buf.begin(), std::min(event_buf.size(), average_tasks - tasks));
                worker->remove_tasks(count);
                local_queue_.enqueue_bulk(event_buf.begin(), count);
                add_tasks(count);
            }
        }
    }
}