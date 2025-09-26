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
    auto transition_to_sleeping() -> bool {
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

    /// Returns `true` if the transition happened.
    [[nodiscard]]
    auto transition_from_sleeping() -> bool {
        if (has_task()) {
            // When a worker wakes, it should only transition to the "searching"
            // state when the wake originates from another worker *or* a new task
            // is pushed. We do *not* want the worker to transition to "searching"
            // when it wakes when the I/O driver receives new events.
            is_searching_ = !shared_.idle_.remove(index_);
            return true;
        }

        if (shared_.idle_.contains(index_)) {
            return false;
        }

        is_searching_ = true;
        return true;
    }

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

    auto should_notify_others() const noexcept -> bool {
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
        if (tick_ % shared_.config_.global_queue_interval == 0) {
            return shared_.next_remote_task().or_else([this] { return next_local_task(); });
        } else {
            if (auto task = next_local_task(); task) {
                return task;
            }

            if (shared_.global_queue_.empty()) {
                return std::nullopt;
            }

            auto n = std::min(local_queue_.remaining_slots(), local_queue_.capacity() / 2);
            if (n == 0) [[unlikely]] {
                // All tasks of current worker are being stolen
                return shared_.next_remote_task();
            }
            // We need pull 1/num_workers part of tasks
            n = std::min(shared_.global_queue_.size() / shared_.workers_.size() + 1, n);
            // n will be set to the number of tasks that are actually fetched
            auto tasks = shared_.global_queue_.pop_n(n);
            if (n == 0) {
                return std::nullopt;
            }
            auto result = std::move(tasks.front());
            tasks.pop_front();
            n -= 1;
            if (n > 0) {
                local_queue_.push_batch(tasks, n);
            }
            return result;
        }
    }

    [[nodiscard]]
    auto next_local_task() -> std::optional<std::coroutine_handle<>> {
        if (lifo_slot_.has_value()) {
            std::optional<std::coroutine_handle<>> result{std::nullopt};
            result.swap(lifo_slot_);
            assert(!lifo_slot_.has_value());
            return result;
        }
        return local_queue_.pop();
    }

    [[nodiscard]]
    auto steal_task() -> std::optional<std::coroutine_handle<>> {
        // Avoid to many worker stealing at same time
        if (!transition_to_searching()) {
            return std::nullopt;
        }
        auto num = shared_.workers_.size();
        // auto start = rand() % num;
        auto start = static_cast<std::size_t>(rand_.fastrand_n(static_cast<uint32_t>(num)));
        for (std::size_t i = 0; i < num; ++i) {
            auto idx = (start + i) % num;
            if (idx == index_) {
                continue;
            }
            if (auto result = shared_.workers_[idx]->local_queue_.steal_into(local_queue_);
                result) {
                return result;
                }
        }
        // Final check the global queue again
        return shared_.next_remote_task();
    }

    void maintenance() {
        if (this->tick_ % shared_.config_.io_interval == 0) {
            [[maybe_unused]] auto _ = poll();
            check_shutdown();
        }
    }

    void check_shutdown() {
        if (!is_shutdown_) [[likely]] {
            is_shutdown_ = shared_.global_queue_.is_closed();
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
        check_shutdown();
        if (transition_to_sleeping()) {
            while (!is_shutdown_) {
                driver_.wait(local_queue_, shared_.global_queue_);
                check_shutdown();
                if (transition_from_sleeping()) {
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
} // namespace kosio::runtime::scheduler::multi_thread
