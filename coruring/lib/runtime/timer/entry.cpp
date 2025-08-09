#include "runtime/timer/entry.h"

void coruring::runtime::timer::Entry::execute() const {
    // 1. 事件被worker放入本地或全局队列
    //    data_ 会被设置为 nullptr，此时事件无法被定时器取消
    // 2. 事件到期还未被放入本地或全局队列，则会提交取消请求
    //    同时将 entry_ 设置为 nullptr，防止被放入本地或全局队列
    if (data_) {
        if (auto sqe = io::t_ring->get_sqe()) [[likely]] {
            io_uring_prep_cancel(sqe, data_, 0);
            io_uring_sqe_set_data(sqe, nullptr);
            io::t_ring->pend_submit();
        }
        data_->entry_ = nullptr;
    }
}
