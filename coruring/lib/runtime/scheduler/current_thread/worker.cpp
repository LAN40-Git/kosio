#include "runtime/scheduler/current_thread/worker.h"
#include "runtime/scheduler/current_thread/handle.h"

coruring::runtime::scheduler::current_thread::Worker::Worker(Handle *handle, const runtime::detail::Config &config)
    : handle_(handle)
    , driver_(config) {
    t_worker = this;
}

void coruring::runtime::scheduler::current_thread::Worker::run() {
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

void coruring::runtime::scheduler::current_thread::Worker::shutdown() {
    is_shutdown_ = true;
    wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::wake_up() const {
    driver_.wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::schedule_local(std::coroutine_handle<> task) {
    if (lifo_slot_.has_value()) {
        local_queue_.push(std::move(lifo_slot_.value()));
        lifo_slot_.emplace(std::move(task));
    } else {
        lifo_slot_.emplace(std::move(task));
    }
}

void coruring::runtime::scheduler::current_thread::Worker::schedule_remote(std::coroutine_handle<> task) {
    global_queue_.push(std::move(task));
    wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::schedule_remote_batch(
    std::list<std::coroutine_handle<>> &&handles, [[maybe_unused]] std::size_t n) {
    global_queue_.push_batch(std::move(handles));
    wake_up();
}

void coruring::runtime::scheduler::current_thread::Worker::run_task(std::coroutine_handle<> task) {
    task.resume();
}

void coruring::runtime::scheduler::current_thread::Worker::tick() {
    tick_ += 1;
}

void coruring::runtime::scheduler::current_thread::Worker::maintenance() {
    if (this->tick_ % config_.io_interval == 0) {
        [[maybe_unused]] auto _ = poll();
    }
}

auto coruring::runtime::scheduler::current_thread::Worker::next_task()
-> std::optional<std::coroutine_handle<>> {
    if (tick_ % config_.global_queue_interval == 0) {
        return next_remote_task().or_else([this] {
            return next_local_task();
        });
    } else {
        if (auto task = next_local_task(); task) {
            return task;
        }

        if (global_queue_.empty()) {
            return std::nullopt;
        }

        auto handles = global_queue_.pop_all();

        if (handles.empty()) {
            return std::nullopt;
        }

        local_queue_.push_batch(handles);
        return next_local_task();
    }
}

auto coruring::runtime::scheduler::current_thread::Worker::next_local_task()
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

auto coruring::runtime::scheduler::current_thread::Worker::next_remote_task()
-> std::optional<std::coroutine_handle<>> {
    if (global_queue_.empty()) {
        return std::nullopt;
    }

    return global_queue_.pop();
}

auto coruring::runtime::scheduler::current_thread::Worker::poll() -> bool {
    if (!driver_.poll(local_queue_, global_queue_)) {
        return false;
    }
    return true;
}

void coruring::runtime::scheduler::current_thread::Worker::sleep() {
    while (!is_shutdown_) [[likely]] {
        driver_.wait(local_queue_, global_queue_);
        if (!local_queue_.empty() || !global_queue_.empty()) {
            break;
        }
    }
}
