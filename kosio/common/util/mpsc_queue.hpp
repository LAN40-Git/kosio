#pragma once
#include "kosio/sync/mutex.hpp"
#include "kosio/sync/condition_variable.hpp"
#include "kosio/common/error.hpp"
#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#include "kosio/third_party/concurrentqueue/concurrentqueue.h"
#endif

namespace kosio::util {
template <typename T>
class MPSCQueue {
public:
    MPSCQueue() = default;
    ~MPSCQueue() = default;

    // Delete copy
    MPSCQueue(const MPSCQueue&) = delete;
    MPSCQueue& operator=(const MPSCQueue&) = delete;

    // Delete move
    MPSCQueue(MPSCQueue&&) = delete;
    MPSCQueue& operator=(MPSCQueue&&) = delete;

public:
    void push(T&& value) {
        queue_.enqueue(std::move(value));
        cv_.notify_one();
    }

    [[REMEMBER_CO_AWAIT]]
    auto pop() -> kosio::async::Task<Result<T>> {
        T item;
        if (queue_.try_dequeue(item)) {
            co_return item;
        }
        co_await mutex_.lock();
        std::unique_lock lock{mutex_, std::adopt_lock};
        while (!queue_.try_dequeue(item)) {
            co_await cv_.wait(mutex_, [this]() {
                return queue_.size_approx() != 0 || is_shutdown_.load(std::memory_order_relaxed);
            });
            if (is_shutdown_.load(std::memory_order_relaxed)) {
                co_return std::unexpected{make_error(Error::kEmptyConcurrentQueue)};
            }
        }
        co_return item;
    }

    [[nodiscard]]
    auto size_approx() -> std::size_t {
        return queue_.size_approx();
    }

    void shutdown() {
        is_shutdown_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

private:
    moodycamel::ConcurrentQueue<T> queue_;
    sync::Mutex                    mutex_;
    sync::ConditionVariable        cv_;
    std::atomic<bool>              is_shutdown_{false};
};
} // namespace kosio::util