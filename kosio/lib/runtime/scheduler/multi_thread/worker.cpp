#include "kosio/include/runtime/scheduler/multi_thread/worker.h"
#include "kosio/include/runtime/scheduler/multi_thread/handle.h"

kosio::runtime::scheduler::multi_thread::Worker::Worker(
    std::size_t index,
    Handle* handle,
    const runtime::detail::Config &config)
    : index_(index)
    , handle_(handle)
    , driver_(config) {
    handle_->shared_.workers_.push_back(this);
    t_worker = this;
    t_shared = std::addressof(handle_->shared_);
}

kosio::runtime::scheduler::multi_thread::Worker::~Worker() {
    t_worker = nullptr;
    t_shared = nullptr;
    handle_->shared_.shutdown_.arrive_and_wait();
}

void kosio::runtime::scheduler::multi_thread::Worker::run() {
    while (!is_shutdown_) [[likely]] {
        tick();

        maintenance();

        if (auto task = next_task(); task) {
            run_task(task.value());
            continue;
        }

        if (auto task = steal_task(); task) {
            run_task(task.value());
            continue;
        }

        if (poll()) {
            continue;
        }

        sleep();
    }
}

void kosio::runtime::scheduler::multi_thread::Worker::shutdown() {
    is_shutdown_ = true;
}

void kosio::runtime::scheduler::multi_thread::Worker::wake_up() const {
    driver_.wake_up();
}

void kosio::runtime::scheduler::multi_thread::Worker::schedule_local(std::coroutine_handle<> task) {
    if (lifo_slot_.has_value()) {
        local_queue_.push_back_or_overflow(std::move(lifo_slot_.value()), handle_->shared_.global_queue_);
        lifo_slot_.emplace(std::move(task));
        handle_->shared_.wake_up_one();
    } else {
        lifo_slot_.emplace(std::move(task));
    }
}

void kosio::runtime::scheduler::multi_thread::Worker::run_task(std::coroutine_handle<> task) {
    this->transition_from_searching();
    task.resume();
}

auto kosio::runtime::scheduler::multi_thread::Worker::transition_to_sleepling() -> bool {
    if (has_task()) {
        return false;
    }

    auto is_last_searcher = handle_->shared_.idle_.transition_worker_to_sleeping(index_, is_searching_);

    is_searching_ = false;
    if (is_last_searcher) {
        handle_->shared_.wake_up_if_work_pending();
    }
    return true;
}

auto kosio::runtime::scheduler::multi_thread::Worker::transition_from_sleepling() -> bool {
    if (has_task()) {
        is_searching_ = !handle_->shared_.idle_.remove(index_);
        return true;
    }

    if (handle_->shared_.idle_.contains(index_)) {
        return false;
    }

    is_searching_ = true;
    return true;
}

auto kosio::runtime::scheduler::multi_thread::Worker::transition_to_searching() -> bool {
    if (!is_searching_) {
        is_searching_ = handle_->shared_.idle_.transition_worker_to_searching();
    }
    return is_searching_;
}

void kosio::runtime::scheduler::multi_thread::Worker::transition_from_searching() {
    if (!is_searching_) {
        return;
    }
    is_searching_ = false;
    // Wake up a sleeping worker, if need
    if (handle_->shared_.idle_.transition_worker_from_searching()) {
        handle_->shared_.wake_up_one();
    }
}

auto kosio::runtime::scheduler::multi_thread::Worker::has_task() const -> bool {
    return lifo_slot_.has_value() || !local_queue_.empty();
}

auto kosio::runtime::scheduler::multi_thread::Worker::should_notify_others() const -> bool {
    if (is_searching_) {
        return false;
    }
    return local_queue_.size() > 1;
}

void kosio::runtime::scheduler::multi_thread::Worker::tick() {
    tick_ += 1;
}

auto kosio::runtime::scheduler::multi_thread::Worker::next_task()
-> std::optional<std::coroutine_handle<>> {
    // 1. 每隔（global_queue_interval）次就尝试从全局队列获取任务
    // 2. 尝试从本地队列取出任务，若本地队列为空，则尝试从全局队列取出至少一个任务
    if (tick_ % handle_->shared_.config_.global_queue_interval == 0) {
        return next_remote_task().or_else([this] {
            return next_local_task();
        });
    } else {
        if (auto task = next_local_task(); task) {
            return task;
        }

        auto& global_queue = handle_->shared_.global_queue_;

        if (global_queue.empty()) {
            return std::nullopt;
        }

        auto n = std::min(local_queue_.remaining_slots(), local_queue_.capacity() / 2);
        if (n == 0) [[unlikely]] {
            // All tasks of current worker are being stolen
            return next_remote_task();
        }
        n = std::min(handle_->shared_.global_queue_.size() / handle_->shared_.workers_.size() + 1, n);
        auto tasks = handle_->shared_.global_queue_.pop_n(n);
        if (n == 0) {
            return std::nullopt;
        }
        local_queue_.push_batch(tasks, n);
        return next_local_task();
    }
}

auto kosio::runtime::scheduler::multi_thread::Worker::next_local_task()
-> std::optional<std::coroutine_handle<>> {
    if (lifo_slot_.has_value()) {
        std::optional<std::coroutine_handle<>> result{std::nullopt};
        result.swap(lifo_slot_);
        return result;
    }

    if (local_queue_.empty()) {
        return std::nullopt;
    }

    return local_queue_.pop();
}

auto kosio::runtime::scheduler::multi_thread::Worker::next_remote_task()
const -> std::optional<std::coroutine_handle<>> {
    auto& global_queue = handle_->shared_.global_queue_;
    if (global_queue.empty()) {
        return std::nullopt;
    }

    return global_queue.pop();
}

auto kosio::runtime::scheduler::multi_thread::Worker::steal_task()
-> std::optional<std::coroutine_handle<>> {
    if (!transition_to_searching()) {
        return std::nullopt;
    }

    auto num = handle_->shared_.workers_.size();
    auto start = static_cast<std::size_t>(rand_.fastrand_n(static_cast<uint32_t>(num)));
    for (std::size_t i = 0; i < num; ++i) {
        i = (start + i) % num;
        if (i == index_) {
            continue;
        }

        // TODO: 动态调整窃取的任务数量
        if (auto result =
            handle_->shared_.workers_[i]->local_queue_.steal_into(local_queue_); result) {
            return result;
        }
    }
    return next_remote_task();
}

void kosio::runtime::scheduler::multi_thread::Worker::maintenance() {
    if (this->tick_ % handle_->shared_.config_.io_interval == 0) {
        [[maybe_unused]] auto _ = poll();
    }
}

auto kosio::runtime::scheduler::multi_thread::Worker::poll() -> bool {
    if (!driver_.poll(local_queue_, handle_->shared_.global_queue_)) {
        return false;
    }
    if (should_notify_others()) {
        handle_->shared_.wake_up_one();
    }
    return true;
}

void kosio::runtime::scheduler::multi_thread::Worker::sleep() {
    if (transition_to_sleepling()) {
        while (!is_shutdown_) [[likely]] {
            driver_.wait(local_queue_, handle_->shared_.global_queue_);
            if (transition_from_sleepling()) {
                break;
            }
        }
    }
}
