#pragma once
#include "kosio/common/util/noncopyable.hpp"
#include "kosio/runtime/scheduler/driver.hpp"
#include "kosio/runtime/scheduler/current_thread/queue.hpp"

namespace kosio::runtime::scheduler::current_thread {
class Worker;
class Handle;
inline thread_local Worker *t_worker{nullptr};

class Worker : util::Noncopyable {
public:
    Worker(Handle* handle, const runtime::detail::Config &config)
    : handle_(handle)
    , driver_(config) {
        t_worker = this;
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

            if (poll()) {
                continue;
            }

            sleep();
        }
    }

    void shutdown() {
        is_shutdown_ = true;
        wake_up();
    }

    void wake_up() const {
        driver_.wake_up();
    }

    void schedule_local(std::coroutine_handle<> task) {
        if (lifo_slot_.has_value()) {
            local_queue_.push(std::move(lifo_slot_.value()));
            lifo_slot_.emplace(std::move(task));
        } else {
            lifo_slot_.emplace(std::move(task));
        }
    }

    void schedule_remote(std::coroutine_handle<> task) {
        global_queue_.push(std::move(task));
        wake_up();
    }

    void schedule_remote_batch(std::list<std::coroutine_handle<>> &&handles, [[maybe_unused]] std::size_t n) {
        global_queue_.push_batch(std::move(handles));
        wake_up();
    }

private:
    void run_task(std::coroutine_handle<> task) {
        task.resume();
    }

    void tick() {
        tick_ += 1;
    }

    void maintenance() {
        if (this->tick_ % config_.io_interval == 0) {
            [[maybe_unused]] auto _ = poll();
        }
    }

    [[nodiscard]]
    auto next_task() -> std::optional<std::coroutine_handle<>> {
        if (tick_ % config_.global_queue_interval == 0) {
            return global_queue_.pop().or_else([this] { return next_local_task(); });
        } else {
            if (auto task = next_local_task(); task) {
                return task;
            }

            auto handles = global_queue_.pop_all();

            if (handles.empty()) {
                return std::nullopt;
            }
            auto result = std::move(handles.front());
            handles.pop_front();

            local_queue_.push_batch(handles);
            return result;
        }
    }

    [[nodiscard]]
    auto next_local_task() -> std::optional<std::coroutine_handle<>> {
        if (lifo_slot_.has_value()) {
            std::optional<std::coroutine_handle<>> result{std::nullopt};
            result.swap(lifo_slot_);
            return result;
        }
        return local_queue_.pop();
    }

    [[nodiscard]]
    auto next_remote_task() -> std::optional<std::coroutine_handle<>> {
        if (global_queue_.empty()) {
            return std::nullopt;
        }

        return global_queue_.pop();
    }

    [[nodiscard]]
    auto poll() -> bool {
        if (!driver_.poll(local_queue_, global_queue_)) {
            return false;
        }
        return true;
    }

    void check_shutdown() {
        if (!is_shutdown_) [[likely]] {
            is_shutdown_ = global_queue_.is_closed();
        }
    }

    void sleep() {
        check_shutdown();
        while (!is_shutdown_) [[likely]] {
            driver_.wait(local_queue_, global_queue_);
            check_shutdown();
            if (!local_queue_.empty() || !global_queue_.empty()) {
                break;
            }
        }
    }

private:
    uint32_t                tick_{0};
    Handle*                 handle_;
    runtime::detail::Config config_;
    detail::Driver          driver_;
    std::optional<std::coroutine_handle<>> lifo_slot_;
    LocalQueue              local_queue_;
    GlobalQueue             global_queue_;
    bool                    is_shutdown_{false};
    bool                    is_searching_{false};
};
} // namespace kosio::runtime::scheduler::current_thread
