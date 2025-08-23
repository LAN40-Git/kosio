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

        // 在必要时轮询完成的 IO 事件
        maintenance();

        // 取出本地任务
        take_tasks();

        // 窃取任务
        steal_tasks();

        // 处理任务
        handle_tasks();

        // 轮询 IO 事件
        if (poll()) {
            continue;
        }

        // 睡眠
        sleep();
    }
}

void coruring::runtime::scheduler::multi_thread::Worker::shutdown() {
    is_shutdown_ = true;
}

void coruring::runtime::scheduler::multi_thread::Worker::wake_up() const {
    driver_.wake_up();
}

auto coruring::runtime::scheduler::multi_thread::Worker::local_queue() -> LocalQueue & {
    return local_queue_;
}

void coruring::runtime::scheduler::multi_thread::Worker::handle_tasks() const {
    auto& tasks = task_remain_.tasks;
    for (std::size_t i = 0; i < task_remain_.index; ++i) {
        tasks[i].resume();
        // 最外层的协程完成了（tasks_ 成功移除），说明任务完成，销毁协程
        if (tasks[i].done() && handle_->tasks_.erase(tasks[i])) {
            tasks[i].destroy();
        }
    }
}

auto coruring::runtime::scheduler::multi_thread::Worker::transition_to_sleepling() -> bool {
    if (local_queue_.size_approx() > 0) {
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
    if (local_queue_.size_approx() > 0) {
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
}

void coruring::runtime::scheduler::multi_thread::Worker::take_tasks() {
    auto count = local_queue_.try_dequeue_bulk(
        task_remain_.tasks.data(), task_remain_.size);
    task_remain_.size -= count;
    task_remain_.index += count;
}

void coruring::runtime::scheduler::multi_thread::Worker::steal_tasks() {
    // 当本轮已经完成足够任务时，不进行窃取
    // 当太多线程在窃取时，不进行窃取
    if (task_remain_.size == 0 || !transition_to_searching()) {
        return;
    }

    auto& global_queue = handle_->shared_.global_queue_;
    // 先从全局队列窃取
    auto steal_count = global_queue.try_dequeue_bulk(
        task_remain_.tasks.data() + task_remain_.index, task_remain_.size);
    task_remain_.size -= steal_count;
    task_remain_.index += steal_count;

    // 若还未拿到足够的任务，则再从其它线程窃取
    if (task_remain_.size == 0) {
        return;
    }
    auto target_index =
        util::FastRand::instance().fastrand_n(handle_->shared_.workers_.size());

    if (target_index == index_) {
        return;
    }

    auto& target_worker = handle_->shared_.workers_[target_index];

    steal_count = target_worker->local_queue().try_dequeue_bulk(
        task_remain_.tasks.data() + task_remain_.index, task_remain_.size);
    task_remain_.size -= steal_count;
    task_remain_.index += steal_count;
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
