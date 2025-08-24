#pragma once
#include "runtime/config.h"
#include "runtime/io/io_uring.h"
#include "runtime/timer/timer.h"
#include "runtime/task/waker.h"

namespace coruring::runtime::scheduler::detail {
class Driver;

inline thread_local Driver* t_driver{nullptr};

class Driver {
public:
    explicit Driver(const runtime::detail::Config& config);
    ~Driver();

public:
    template <typename LocalQueue>
    void wait(LocalQueue &local_queue) {
        ring_.wait(timer_.next_expiration_time());
        poll(local_queue);
    }

    template <typename LocalQueue>
    auto poll(LocalQueue &local_queue) -> bool {
        std::array<io_uring_cqe *, runtime::detail::PEEK_BATCH_SIZE> cqes{};
        auto count = ring_.peek_batch(cqes);
        for (auto i = 0; i < count; ++i) {
            auto cb = reinterpret_cast<coruring::io::detail::Callback *>(cqes[i]->user_data);
            if (cb) [[likely]] {
                // 若 cb->entry_ != nullptr，说明事件还未被 timer 取消，
                // 那么将事件放入本地队列
                if (cb->entry_) {
                    // 将事件标从分层时间轮中移除
                    timer::Timer::remove(cb->entry_);
                }
                cb->result_ = cqes[i]->res;
                local_queue.enqueue(std::move(cb->handle_));
            }
        }

        ring_.consume(count);

        count += timer_.handle_expired_entries(util::current_ms() - timer_.start_time());

        waker_.turn_on();

        ring_.submit();

        return count > 0;
    }

    void wake_up() const;

private:
    io::IoUring        ring_;
    timer::Timer       timer_;
    task::waker::Waker waker_;
};
} // namespace coruring::runtime::scheduler
