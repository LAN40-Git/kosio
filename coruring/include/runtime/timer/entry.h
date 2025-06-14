#pragma once
#include <iostream>

#include "runtime/io/io_uring.h"
#include "io/base/callback.h"

namespace coruring::runtime::detail
{
class Entry {
public:
    explicit Entry(io::detail::Callback *data, int64_t expiration_ms)
        : data_{data}
        , expiration_ms_{expiration_ms} {}

public:
    void execute() {
        if (data_) {
            // 若操作未完成则提交取消请求
            auto sqe = IoUring::instance().get_sqe();
            // TODO: 去除检查
            if (sqe) [[likely]] {
                io_uring_prep_cancel(sqe, data_, 0);
                io_uring_sqe_set_data(sqe, nullptr);
                IoUring::instance().pend_submit();
            }
            data_->entry_ = nullptr;
        }
    }

public:
    io::detail::Callback *data_{};          // 提交到 io_uring 的数据
    int64_t               expiration_ms_{}; // 剩余时间（毫秒）
};
}
