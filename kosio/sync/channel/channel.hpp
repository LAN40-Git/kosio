#pragma once
#include "kosio/sync/mutex.hpp"
#include "kosio/third_party/concurrentqueue/concurrentqueue.h"
#include <queue>
#include <coroutine>

namespace kosio::sync::detail {
template <typename T>
class Channel {
    struct SendAwaiter {
        SendAwaiter(Channel &channel, T &value)
            : channel_{channel}
        , value_{value} {}

        auto await_ready() const noexcept -> bool {
            return false;
        }

        auto await_suspend(std::coroutine_handle<> handle) -> bool {
            {
                std::lock_guard lock(channel_.mutex_, std::adopt_lock);
                handle_ = handle;
                if (!channel_.recv_waiters_.empty()) {
                    auto receiver = channel_.recv_waiters_.front();
                    channel_.recv_waiters_.pop();
                    receiver->result_ = std::move(value_);
                    runtime::detail::schedule_local(receiver->handle_);
                    result_.emplace();
                    return false;
                }
            }
            if (!channel_.buffer_.enqueue(std::move(value_))) {
                if (channel_.is_closed_.load(std::memory_order_relaxed)) {
                    return false;
                }
                channel_.send_waiters_.push(this);
                return true;
            } else {
                result_.emplace();
                return false;
            }
        }

        auto await_resum() const noexcept {
            return result_;
        }

    public:
        Channel                &channel_;
        T                      &value_;
        Result<void>            result_;
        std::coroutine_handle<> handle_;
    };

    struct RecvAwaiter {
        RecvAwaiter(Channel& channel)
            : channel_{channel} {}

        auto await_ready() const noexcept -> bool {
            return false;
        }

        auto await_suspend(std::coroutine_handle<> handle) -> bool {
            {
                std::lock_guard lock(channel_.mutex_, std::adopt_lock);
                if (!channel_.send_waiters_.empty()) {
                    auto sender = channel_.send_waiters_.front();
                    channel_.send_waiters_.pop();
                    sender.result_.emplace();
                    result_.emplace(std::move(sender.value_));
                    runtime::detail::schedule_local(sender.handle_);
                    return false;
                }
            }
            if (channel_.buffer_.size_approx() == 0) {
                if (channel_.is_closed_.load(std::memory_order_relaxed)) {
                    return false;
                }
                channel_.recv_waiters_.push(this);
                return true;
            } else {
                T result;
                channel_.buffer_.try_dequeue(result);
                result_.emplace(std::move(result));
                return false;
            }
        }

    public:
        Channel                &channel_;
        Result<T>    result_;
        std::coroutine_handle<> handle_;
    };

public:
    auto send(T value) -> async::Task<Result<void>> {
        co_await mutex_.lock();
        co_return co_await SendAwaiter{*this, value};
    }

    auto recv() -> async::Task<Result<T>> {
        co_await mutex_.lock();
        co_return co_await RecvAwaiter{*this};
    }

private:
    void destroy() {
        if (is_closed_.exchange(true, std::memory_order::acq_rel) == false) {
            for (auto sender : send_waiters_) {
                runtime::detail::schedule_local(sender->handle_);
                send_waiters_.pop();
            }
            for (auto receiver : recv_waiters_) {
                runtime::detail::schedule_local(receiver->handle_);
                recv_waiters_.pop();
            }
        }
    }

private:
    moodycamel::ConcurrentQueue<T> buffer_;
    std::atomic<bool>              is_closed_{false};
    Mutex                          mutex_;
    std::queue<SendAwaiter>        send_waiters_;
    std::queue<RecvAwaiter>        recv_waiters_;
};
} // namespace kosio::sync::detail