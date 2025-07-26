#pragma once
#include <tbb/concurrent_hash_map.h>
#include "multi_thread/worker.h"

namespace coruring::runtime::detail {
class Scheduler : public util::Noncopyable {
    using Worker = std::unique_ptr<multi_thread::detail::Worker>;
    using TaskQueue = moodycamel::ConcurrentQueue<std::coroutine_handle<>>;
    using Handles = tbb::concurrent_hash_map<std::coroutine_handle<>, void*>;
public:
    explicit Scheduler(int32_t worker_nums)
        : worker_nums_(worker_nums)
        , config_(Config::load()) {
        assert(worker_nums > 0);
        handles_.rehash(config_.entries);
    }
    ~Scheduler() { stop(); }

public:
    void run();
    void stop();
    [[nodiscard]]
    bool is_running() const { return is_running_; }

public:
    void spawn(async::Task<void>&& task) noexcept {
        auto handle = task.take();
        handles_.emplace(handle, nullptr);
        global_queue_.enqueue(handle);
    }

public:
    [[nodiscard]]
    auto workers() -> std::vector<Worker>& { return workers_; }
    [[nodiscard]]
    auto worker_nums() const -> int32_t { return worker_nums_; }
    [[nodiscard]]
    auto global_queue() -> TaskQueue& { return global_queue_; }
    [[nodiscard]]
    auto handles() -> Handles& { return handles_; }

private:
    std::atomic<bool>   is_running_{false};
    std::mutex          mutex_;
    int32_t             worker_nums_;
    std::vector<Worker> workers_;
    TaskQueue           global_queue_;
    Handles             handles_;
    const Config&       config_;
};
} // namespace coruring::runtime::detail
