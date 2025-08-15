#include "runtime/timer/entry.h"

void coruring::runtime::timer::Entry::execute() const {
    // 若 io 事件已完成，则 data_ 会被设置为 nullptr
    // 此时不需要进行操作
    if (data_) {
        // 若 io 事件未完成，则提交取消请求
        if (auto sqe = io::t_ring->get_sqe()) [[likely]] {
            io_uring_prep_cancel(sqe, data_, 0);
            io_uring_sqe_set_data(sqe, nullptr);
            io::t_ring->pend_submit();
        }
        // 将 entry_ 设置为 nullptr
        // 告知 worker 不需要将事件从分层时间轮中移除
        data_->entry_ = nullptr;
    }
}
