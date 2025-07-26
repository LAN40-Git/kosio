#pragma once
#include <iostream>

#include "runtime/io/io_uring.h"
#include "io/base/callback.h"

namespace coruring::runtime::detail {
class Entry {
public:
    explicit Entry(io::detail::Callback *data, uint64_t expiration_ms)
        : data_{data}
        , expiration_ms_{expiration_ms} {}

public:
    void execute();

public:
    io::detail::Callback *data_{};          // 提交到 io_uring 的数据
    uint64_t              expiration_ms_{}; // 绝对超时时间（毫秒）
};
} // namespace coruring::runtime::detail
