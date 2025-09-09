#pragma once
#include "kosio/runtime/config.hpp"
#include "kosio/runtime/scheduler/multi_thread/shared.hpp"
#include "kosio/runtime/scheduler/multi_thread/worker.hpp"

namespace kosio::runtime::scheduler::multi_thread {
class Handle {
public:
    explicit Handle(const runtime::detail::Config& config)
        : shared_(config) {
        for (std::size_t i = 0; i < config.num_workers; i++) {
            std::latch sync{2};
            auto thread_name = "kosio-WORKER-" + std::to_string(i);
            threads_.emplace_back([i, this, config, &thread_name, &sync]() {
                Worker worker{i, this, config};

                util::set_current_thread_name(thread_name);

                sync.count_down();

                worker.run();
            });
            sync.arrive_and_wait();
        }
    }

    ~Handle() {
        shared_.close();
        for (auto &thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

public:
    void schedule_task(std::coroutine_handle<> task) {
        shared_.schedule_remote(task);
    }

    void close() const {
        shared_.close();
    }

    void wait() {
        for (auto &thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

public:
    std::vector<std::thread> threads_;
    Shared                   shared_;
};

static inline void schedule_local(std::coroutine_handle<> handle) {
    if (t_worker == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_worker->schedule_local(handle);
}

static inline void schedule_remote(std::coroutine_handle<> handle) {
    if (t_shared == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_shared->schedule_remote(handle);
}

template <typename It>
static inline void schedule_remote_batch(It itemFirst, std::size_t count) {
    if (t_shared == nullptr) [[unlikely]] {
        std::unreachable();
        return;
    }
    t_shared->schedule_remote_batch(itemFirst, count);
}
} // namespace kosio::runtime::scheduler::multi_thread
