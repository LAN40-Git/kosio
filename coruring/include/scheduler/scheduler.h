#pragma once
#include <set>
#include "runtime/worker/worker.h"

namespace coruring::scheduler
{
class Scheduler : public util::Noncopyable {
    using Worker = std::unique_ptr<runtime::detail::Worker>;
public:
    explicit Scheduler(std::size_t worker_nums)
        : worker_nums_(worker_nums) {}
    ~Scheduler() { stop();clear(); }

public:
    void run();
    void stop();
    [[nodiscard]]
    bool is_running() const { return is_running_; }

public:
    template <typename T>
    void spawn(async::Task<T>&& task) noexcept {
        auto handle = task.take();
        global_queue_.enqueue(handle);
    }
    template <typename It>
        requires std::input_iterator<It> &&
         std::convertible_to<std::iter_value_t<It>, std::coroutine_handle<>>
    void spawn_batch(It itemFirst, size_t count) {
        global_queue_.enqueue_bulk(itemFirst, count);
    }

public:
    auto workers() -> std::set<Worker>& { return workers_; }
    auto global_queue() -> runtime::detail::Worker::TaskQueue& { return global_queue_; }

private:
    void clear() noexcept;

private:
    std::atomic<bool>                  is_running_{false};
    std::mutex                         mutex_;
    std::size_t                        worker_nums_;
    std::set<Worker>                   workers_;
    runtime::detail::Worker::IoBuf     io_buf_;
    runtime::detail::Worker::TaskQueue global_queue_;
};
}
