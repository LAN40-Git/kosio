#pragma once
#include <tbb/concurrent_hash_map.h>
#include "runtime/worker/worker.h"

namespace coruring::scheduler
{
class Scheduler : public util::Noncopyable {
    using Worker = std::unique_ptr<runtime::detail::Worker>;
    using TaskQueue = moodycamel::ConcurrentQueue<std::coroutine_handle<>>;
    using IoBuf = std::array<std::coroutine_handle<>, runtime::detail::Config::IO_BATCH_SIZE>;
    using Handles = tbb::concurrent_hash_map<std::coroutine_handle<>, void*>;
public:
    explicit Scheduler(std::size_t worker_nums)
        : worker_nums_(worker_nums)
        , config_(runtime::detail::Config::load()) { assert(worker_nums > 0); }
    ~Scheduler() { stop(); }

public:
    void run();
    void stop();
    [[nodiscard]]
    bool is_running() const { return is_running_; }

public:
    template <typename T>
    void spawn(async::Task<T>&& task) noexcept {
        auto handle = task.take();
        handles_.emplace(handle, nullptr);
        global_queue_.enqueue(handle);
    }

public:
    auto workers() -> std::vector<Worker>& { return workers_; }
    auto worker_nums() const -> std::size_t { return worker_nums_; }
    auto global_queue() -> TaskQueue& { return global_queue_; }
    auto handle_set() -> Handles& { return handles_; }

private:
    void clear() noexcept;

private:
    std::atomic<bool>              is_running_{false};
    std::mutex                     mutex_;
    std::size_t                    worker_nums_;
    std::vector<Worker>            workers_;
    IoBuf                          io_buf_;
    TaskQueue                      global_queue_;
    Handles                        handles_;
    const runtime::detail::Config& config_;
};
}
