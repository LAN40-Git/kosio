#pragma once
#include "kosio/common/util/random.hpp"
#include "kosio/common/util/noncopyable.hpp"
#include "kosio/runtime/scheduler/driver.hpp"
#include "kosio/runtime/scheduler/multi_thread/shared.hpp"

namespace kosio::runtime::scheduler::multi_thread {
class Handle;
inline thread_local Worker *t_worker{nullptr};

class Worker : util::Noncopyable {
    friend class Shared;
public:
    Worker(std::size_t index, Shared& shared)
        : index_(index)
        , shared_(shared)
        , driver_(shared_.config_) {
        shared_.workers_.push_back(this);
        t_worker = this;
        t_shared = std::addressof(shared);
    }

    ~Worker() {
        t_worker = nullptr;
        t_shared = nullptr;
        shared_.shutdown_.arrive_and_wait();
    }

public:
    void run() {
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

    void shutdown() {
        is_shutdown_ = true;
    }

    void wake_up() const {
        driver_.wake_up();
    }

    void schedule_local(std::coroutine_handle<> task) {
        if (lifo_slot_.has_value()) {
            local_queue_.push_back_or_overflow(std::move(lifo_slot_.value()), shared_.global_queue_);
            lifo_slot_.emplace(std::move(task));
            shared_.wake_up_one();
        } else {
            lifo_slot_.emplace(std::move(task));
        }
    }

private:
    void run_task(std::coroutine_handle<> task) {
        this->transition_from_searching();
        task.resume();
    }

    [[nodiscard]]
    auto transition_to_sleepling() -> bool {
        if (has_task()) {
            return false;
        }

        auto is_last_searcher = shared_.idle_.transition_worker_to_sleeping(index_, is_searching_);

        is_searching_ = false;
        if (is_last_searcher) {
            shared_.wake_up_if_work_pending();
        }
        return true;
    }

    [[nodiscard]]
    auto transition_from_sleepling() -> bool {
        if (has_task()) {
            is_searching_ = !shared_.idle_.remove(index_);
            return true;
        }

        if (shared_.idle_.contains(index_)) {
            return false;
        }

        is_searching_ = true;
        return true;
    }

    [[nodiscard]]
    auto transition_to_searching() -> bool {
        if (!is_searching_) {
            is_searching_ = shared_.idle_.transition_worker_to_searching();
        }
        return is_searching_;
    }

    void transition_from_searching() {
        if (!is_searching_) {
            return;
        }
        is_searching_ = false;
        // Wake up a sleeping worker, if need
        if (shared_.idle_.transition_worker_from_searching()) {
            shared_.wake_up_one();
        }
    }

    [[nodiscard]]
    auto has_task() const -> bool {
        return lifo_slot_.has_value() || !local_queue_.empty();
    }

    [[nodiscard]]
    auto should_notify_others() const -> bool {
        if (is_searching_) {
            return false;
        }
        return local_queue_.size() > 1;
    }

    void tick() {
        tick_ += 1;
    }

    [[nodiscard]]
    auto next_task() -> std::optional<std::coroutine_handle<>> {
        // 1. 每隔（global_queue_interval）次就尝试从全局队列获取任务
        // 2. 尝试从本地队列取出任务，若本地队列为空，则尝试从全局队列取出至少一个任务
        if (tick_ % shared_.config_.global_queue_interval == 0) {
            return next_remote_task().or_else([this] {
                return next_local_task();
            });
        } else {
            if (auto task = next_local_task(); task) {
                return task;
            }

            auto& global_queue = shared_.global_queue_;

            if (global_queue.empty()) {
                return std::nullopt;
            }

            auto n = std::min(local_queue_.remaining_slots(), local_queue_.capacity() / 2);
            if (n == 0) [[unlikely]] {
                // All tasks of current worker are being stolen
                return next_remote_task();
            }
            n = std::min(shared_.global_queue_.size() / shared_.workers_.size() + 1, n);
            auto tasks = shared_.global_queue_.pop_n(n);
            if (n == 0) {
                return std::nullopt;
            }
            local_queue_.push_batch(tasks, n);
            return next_local_task();
        }
    }

    [[nodiscard]]
    auto next_local_task() -> std::optional<std::coroutine_handle<>> {
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

    [[nodiscard]]
    auto next_remote_task() const -> std::optional<std::coroutine_handle<>> {
        auto& global_queue = shared_.global_queue_;
        if (global_queue.empty()) {
            return std::nullopt;
        }

        return global_queue.pop();
    }

    [[nodiscard]]
    auto steal_task() -> std::optional<std::coroutine_handle<>> {
        if (!transition_to_searching()) {
            return std::nullopt;
        }

        auto num = shared_.workers_.size();
        auto start = static_cast<std::size_t>(rand_.fastrand_n(static_cast<uint32_t>(num)));
        for (std::size_t i = 0; i < num; ++i) {
            i = (start + i) % num;
            if (i == index_) {
                continue;
            }

            // TODO: 动态调整窃取的任务数量
            if (auto result =
                shared_.workers_[i]->local_queue_.steal_into(local_queue_); result) {
                return result;
            }
        }
        return next_remote_task();
    }

    void maintenance() {
        if (this->tick_ % shared_.config_.io_interval == 0) {
            [[maybe_unused]] auto _ = poll();
        }
    }

    [[nodiscard]]
    auto poll() -> bool {
        if (!driver_.poll(local_queue_, shared_.global_queue_)) {
            return false;
        }
        if (should_notify_others()) {
            shared_.wake_up_one();
        }
        return true;
    }

    void sleep() {
        if (transition_to_sleepling()) {
            while (!is_shutdown_) [[likely]] {
                driver_.wait(local_queue_, shared_.global_queue_);
                if (transition_from_sleepling()) {
                    break;
                }
            }
        }
    }

private:
    std::size_t                            index_;
    uint32_t                               tick_{0};
    util::FastRand                         rand_{};
    Shared&                                shared_;
    detail::Driver                         driver_;
    std::optional<std::coroutine_handle<>> lifo_slot_{std::nullopt};
    LocalQueue                             local_queue_;
    bool                                   is_shutdown_{false};
    bool                                   is_searching_{false};
};

inline void Shared::wake_up_one() {
    if (auto index = idle_.worker_to_notify(); index) {
        workers_[index.value()]->wake_up();
    }
}

inline void Shared::wake_up_all() const {
    for (auto& worker : workers_) {
        worker->wake_up();
    }
}

inline void Shared::wake_up_if_work_pending() {
    for (auto& worker : workers_) {
        if (!worker->local_queue_.empty()) {
            worker->wake_up();
            return;
        }
    }
    if (!global_queue_.empty()) {
        wake_up_one();
    }
}

inline void Shared::close() {
    for (auto& worker : workers_) {
        worker->shutdown();
    }
    wake_up_all();
}
} // namespace kosio::runtime::scheduler::multi_thread
