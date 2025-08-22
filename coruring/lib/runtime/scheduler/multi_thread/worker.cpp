#include "runtime/scheduler/multi_thread/worker.h"

coruring::runtime::scheduler::multi_thread::Worker::Worker(
    std::size_t index,
    Handle* handle,
    const runtime::detail::Config &config)
    : index_(index)
    , handle_(handle)
    , driver_(config) {
    handle_->shared_.workers_.push_back(this);
    t_worker = this;
    t_shared = &handle_->shared_;
}

coruring::runtime::scheduler::multi_thread::Worker::~Worker() {
    t_worker = nullptr;
    t_shared = nullptr;
    handle_->shared_.shutdown_.arrive_and_wait();
}

void coruring::runtime::scheduler::multi_thread::Worker::run() {
    while (!is_shutdown_) [[likely]] {
        tick();

        maintenance();

        handle_tasks();

        if (poll()) {
            continue;
        }

        sleep();
    }
}

void coruring::runtime::scheduler::multi_thread::Worker::wake_up() const {
    driver_.wake_up();
}

void coruring::runtime::scheduler::multi_thread::Worker::shutdown() {
    is_shutdown_ = true;
}

void coruring::runtime::scheduler::multi_thread::Worker::handle_tasks() {
    static std::array<std::coroutine_handle<>, runtime::detail::HANDLE_BATCH_SIZE> io_buf;
    auto count = local_queue_.try_dequeue_bulk(io_buf.data(), io_buf.size());
    for (std::size_t i = 0; i < count; ++i) {
        io_buf[i].resume();
        // 最外层的协程完成了（tasks_ 成功移除），说明任务完成，销毁协程
        if (io_buf[i].done() && handle_->tasks_.erase(io_buf[i])) {
            io_buf[i].destroy();
            owned_tasks_.fetch_sub(1);
        }
    }
}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_to_sleepling() -> bool {
    if (owned_tasks_.load(std::memory_order_relaxed) > 0) {
        return false;
    }

    auto is_last_searcher = handle_->shared_.idle_.transition_worker_to_sleeping(index_, is_searching_);

    is_searching_ = false;
    if (is_last_searcher) {
        handle_->shared_.wake_up_if_work_pending();
    }
    return true;
}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_from_sleepling() -> bool {
    if (owned_tasks_.load(std::memory_order_relaxed) > 0) {
        is_searching_ = !handle_->shared_.idle_.remove(index_);
    }

    if (handle_->shared_.idle_.contains(index_)) {
        return false;
    }

    is_searching_ = true;
    return true;
}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_to_searching() -> bool {
    if (!is_searching_) {
        is_searching_ = handle_->shared_.idle_.transition_worker_to_searching();
    }
    return is_searching_;
}

void coruring::runtime::scheduler::multi_thread::Worker::transition_from_searching() {
    if (!is_searching_) {
        return;
    }
    is_searching_ = false;
    // Wake up a sleeping worker, if need
    if (handle_->shared_.idle_.transition_worker_from_searching()) {
        handle_->shared_.wake_up_one();
    }
}

auto coruring::runtime::scheduler::multi_thread::Worker::should_notify_others() const -> bool {
    if (is_searching_) {
        return false;
    }
    return local_queue_.size_approx() > 1;
}

void coruring::runtime::scheduler::multi_thread::Worker::tick() {
    tick_ += 1;
    // Update the average_tasks_
    average_tasks_ = handle_->tasks_.size() / handle_->shared_.workers_.size();
}

auto coruring::runtime::scheduler::multi_thread::Worker::steal_work() -> std::size_t {
    auto target_index = util::FastRand::instance().rand_range(0, handle_->shared_.workers_.size());
}

void coruring::runtime::scheduler::multi_thread::Worker::maintenance() {
    if (this->tick_ % handle_->shared_.config_.io_interval == 0) {
        // Poll happend I/O events, I don't care if I/O events happen
        // Just a regular checking
        [[maybe_unused]] auto _ = poll();
    }
}

auto coruring::runtime::scheduler::multi_thread::Worker::poll() -> bool {
    if (!driver_.poll(local_queue_)) {
        return false;
    }
    if (should_notify_others()) {
        handle_->shared_.wake_up_one();
    }
    return true;
}

void coruring::runtime::scheduler::multi_thread::Worker::sleep() {
    if (transition_to_sleepling()) {
        while (!is_shutdown_) [[likely]] {
            driver_.wait(local_queue_);
            if (transition_from_sleepling()) {
                break;
            }
        }
    }
}
