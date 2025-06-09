#pragma once
#include "io/base/callback.h"

namespace coruring::runtime::detail
{
class Entry {
public:
    explicit Entry(uint64_t expiration_ms, io::detail::Callback *data)
        : expiration_ms_{expiration_ms}
        , data_{data} {}

private:
    uint64_t              expiration_ms_{}; // 剩余时间（毫秒）
    io::detail::Callback *data_{}; // 提交到 io_uring 的数据
};
}
