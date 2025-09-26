#pragma once
#include "kosio/common/error.hpp"

namespace kosio::sync {
template <typename T>
class SPSCQueue {
public:
    [[REMEMBER_CO_AWAIT]]
    auto push(T&& value) -> async::Task<> {
        co_await mutex_.lock();
        std::unique_lock lock{mutex_, std::adopt_lock};
        queue_.push(std::move(value));
        cv_.notify_one();
    }

    [[REMEMBER_CO_AWAIT]]
    auto pop() -> async::Task<Result<T>> {
        co_await mutex_.lock();
        std::unique_lock lock{mutex_, std::adopt_lock};
        while (queue_.empty()) {
            co_await cv_.wait(mutex_, [this]() {
                return !queue_.empty() || is_shutdown_.load(std::memory_order_relaxed);
            });
            if (is_shutdown_.load(std::memory_order_relaxed)) {
                co_return std::unexpected{make_error(Error::kEmptySPSCQueue)};
            }
        }
        T item = std::move(queue_.front());
        queue_.pop();
        co_return item;
    }

    [[nodiscard]]
    auto size() -> std::size_t {
        return queue_.size();
    }

    [[REMEMBER_CO_AWAIT]]
    auto shutdown() -> async::Task<> {
        co_await mutex_.lock();
        std::unique_lock lock{mutex_, std::adopt_lock};
        is_shutdown_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

private:
    std::queue<T>     queue_;
    Mutex             mutex_;
    ConditionVariable cv_;
    std::atomic<bool> is_shutdown_{false};
};
} // namespace kosio::sync