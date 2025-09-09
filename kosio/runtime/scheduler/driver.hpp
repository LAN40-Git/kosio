#pragma once
#include "kosio/runtime/config.hpp"
#include "kosio/runtime/io/io_uring.hpp"
#include "kosio/runtime/timer/timer.hpp"
#include "kosio/runtime/task/waker.hpp"
#include "kosio/runtime/scheduler/multi_thread/queue.hpp"

namespace kosio::runtime::scheduler::detail {
class Driver;

inline thread_local Driver* t_driver{nullptr};

class Driver {
public:
    explicit Driver(const runtime::detail::Config &config)
    : ring_(config) {
        assert(t_driver == nullptr);
        t_driver = this;
    }

    ~Driver() {
        t_driver = nullptr;
    }

public:
    template <typename LocalQueue, typename GlobalQueue>
    void wait(LocalQueue &local_queue, GlobalQueue &global_queue) {
        ring_.wait(timer_.next_expiration_time());
        poll(local_queue, global_queue);
    }

    template <typename LocalQueue, typename GlobalQueue>
    auto poll(LocalQueue &local_queue, GlobalQueue &global_queue) -> bool {
        thread_local std::array<io_uring_cqe *, runtime::detail::LOCAL_QUEUE_CAPACITY> cqes{};
        auto count = ring_.peek_batch(cqes);
        for (auto i = 0; i < count; ++i) {
            auto cb = reinterpret_cast<kosio::io::detail::Callback *>(cqes[i]->user_data);
            if (cb) [[likely]] {
                // 若 cb->entry_ != nullptr，说明事件还未被 timer 取消，
                // 那么将事件放入本地队列
                if (cb->entry_) {
                    // 将事件标从分层时间轮中移除
                    timer_.remove(cb->entry_);
                }
                cb->result_ = cqes[i]->res;
                local_queue.push_back_or_overflow(cb->handle_, global_queue);
            }
        }

        ring_.consume(count);

        auto timer_count = timer_.handle_expired_entries(local_queue, global_queue);

        count += timer_count;

        // LOG_VERBOSE("poll {} events, {} timer events.", count, timer_count);

        waker_.turn_on();

        ring_.submit();

        return count > 0;
    }

    void wake_up() const {
        waker_.wake_up();
    }

private:
    io::IoUring        ring_;
    timer::Timer       timer_;
    task::waker::Waker waker_;
};
} // namespace kosio::runtime::scheduler
