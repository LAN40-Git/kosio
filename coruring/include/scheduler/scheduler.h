#pragma once
#include <set>
#include "runtime/worker/worker.h"

namespace coruring::scheduler
{
class Scheduler : public util::Noncopyable {
    using Worker = std::unique_ptr<runtime::detail::Worker>;
    using TaskQueue = moodycamel::ConcurrentQueue<std::coroutine_handle<>>;
    using IoBuf = std::array<std::coroutine_handle<>, runtime::detail::Config::IO_BATCH_SIZE>;
public:
    explicit Scheduler(std::size_t worker_nums)
        : worker_nums_(worker_nums)
        , config_(runtime::detail::Config::load()) {}
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
        insert(handle);
        global_queue_.enqueue(handle);
    }

    void insert(std::coroutine_handle<> handle) {
        std::lock_guard lock(handle_mutex_);
        handles_.emplace(handle);
    }

    void erase(std::coroutine_handle<> handle) {
        std::lock_guard lock(handle_mutex_);
        handles_.erase(handle);
    }

    // template <typename It>
    //     requires std::input_iterator<It> &&
    //      std::convertible_to<std::iter_value_t<It>, std::coroutine_handle<>>
    // void spawn_batch(It itemFirst, size_t count) {
    //     global_queue_.enqueue_bulk(itemFirst, count);
    // }

public:
    auto workers() -> std::vector<Worker>& { return workers_; }
    auto global_queue() -> TaskQueue& { return global_queue_; }

private:
    void clear() noexcept;

private:
    std::atomic<bool>              is_running_{false};
    std::mutex                     mutex_;
    std::size_t                    worker_nums_;
    std::vector<Worker>            workers_;
    IoBuf                          io_buf_;
    TaskQueue                      global_queue_;
    std::unordered_set<std::coroutine_handle<>> handles_;
    std::mutex handle_mutex_;
    const runtime::detail::Config& config_;
};
}
