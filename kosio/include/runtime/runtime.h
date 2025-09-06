#pragma once
#include "async/coroutine/task.h"
#include "timer/sleep.h"
#include "runtime/builder.h"

namespace kosio::runtime::detail {
template <typename Handle>
class Runtime {
public:
template <typename... Args>
Runtime(Args &&...args)
    : handle_{std::forward<Args>(args)...} {}

public:
    // Waiting for the task to close
    void block_on(async::Task<void> &&task) {
        auto main_coro = [](Handle &handle, async::Task<void> task) -> async::Task<void> {
            try {
                co_await task;
            } catch (const std::exception &ex) {
                // LOG_ERROR("{}", ex.what());
            } catch (...) {
                // LOG_ERROR("Catch a unknown exception");
            }
            handle.close();
            co_return;
        } (handle_, std::move(task));

        handle_.schedule_task(main_coro.take());

        handle_.wait();
    }

    void shutdown_timeout(uint64_t delay) {
        auto task = [](Handle &handle, uint64_t delay) -> async::Task<void> {
            co_await kosio::timer::sleep(delay);
            handle.close();
            co_return {};
        }(handle_, delay);

        handle_.schedule_task(task.take());
    }

private:
    Handle handle_;
};

static inline auto is_current_thread() -> bool {
    return scheduler::current_thread::t_worker != nullptr;
}

static inline void schedule_local(std::coroutine_handle<> handle) {
    if (is_current_thread()) {
        scheduler::current_thread::schedule_local(handle);
    } else {
        scheduler::multi_thread::schedule_local(handle);
    }
}

static inline void schedule_remote(std::coroutine_handle<> handle) {
    if (is_current_thread()) {
        scheduler::current_thread::schedule_remote(handle);
    } else {
        scheduler::multi_thread::schedule_remote(handle);
    }
}
} // namespace kosio::runtime::detail

namespace kosio {
static inline auto spawn(async::Task<void> &&task) {
    auto handle = task.take();
    runtime::detail::schedule_local(handle);
    return handle;
}
}
