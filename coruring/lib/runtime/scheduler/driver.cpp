#include "runtime/scheduler/driver.h"

coruring::runtime::scheduler::detail::Driver::Driver(const runtime::detail::Config &config)
    : ring_(config) {
    assert(t_driver == nullptr);
    t_driver = this;
}

coruring::runtime::scheduler::detail::Driver::~Driver() {
    t_driver = nullptr;
}

void coruring::runtime::scheduler::detail::Driver::wait(TaskQueue &local_queue) {
    ring_.wait(timer_.next_expiration_time());
    poll(local_queue);
}

auto coruring::runtime::scheduler::detail::Driver::poll(TaskQueue &local_queue) -> bool {
    std::array<io_uring_cqe *, runtime::detail::MAX_QUEUE_BATCH_SIZE> cqes{};
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

void coruring::runtime::scheduler::detail::Driver::wake_up() const {
    waker_.wake_up();
}
