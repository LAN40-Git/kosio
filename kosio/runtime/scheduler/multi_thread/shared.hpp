#pragma once
#include "kosio/runtime/scheduler/multi_thread/queue.hpp"
#include "kosio/runtime/scheduler/multi_thread/idle.hpp"
#include <latch>
#include <vector>

namespace kosio::runtime::scheduler::multi_thread {
class Worker;
class Shared;

inline thread_local Shared *t_shared{nullptr};

class Shared {
    friend class Worker;
public:
    explicit Shared(const runtime::detail::Config &config)
        : config_(config)
        , idle_(config.num_workers)
        , shutdown_{static_cast<std::ptrdiff_t>(config.num_workers)} {
        t_shared = this;
    }

    ~Shared() {
        t_shared = nullptr;
    }

public:
    void wake_up_one();
    void wake_up_all() const;
    void wake_up_if_work_pending();

    void close() {
        if (global_queue_.close()) {
            wake_up_all();
        }
    }

    void schedule_remote(std::coroutine_handle<> task) {
        global_queue_.push(std::move(task));
        wake_up_one();
    }

    void schedule_remote_batch(std::list<std::coroutine_handle<>> &&handles, std::size_t n) {
        global_queue_.push_batch(std::move(handles), n);
        wake_up_one();
    }

private:
    const runtime::detail::Config config_;
    Idle                          idle_;
    GlobalQueue                   global_queue_;
    std::latch                    shutdown_;
    std::vector<Worker*>          workers_;
};
} // namespace kosio::runtime::scheduler::multi_thread
