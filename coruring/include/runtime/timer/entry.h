#pragma once
#include "runtime/io/io_uring.h"
#include "io/base/callback.h"

namespace coruring::runtime::timer {
class Entry {
public:
    Entry() = default;
    explicit Entry(io::detail::Callback *data, uint64_t expiration_time)
        : data_{data}
        , expiration_time_{expiration_time} {}

public:
    void execute() const;

public:
    io::detail::Callback *data_{};            // 提交到 io_uring 的数据
    uint64_t              expiration_time_{}; // 绝对超时时间（毫秒）
};

namespace detail {
// Only used in wheel
using EntryList = std::array<Entry, runtime::detail::LEVEL_MULT>;
} // namespace detail
} // namespace coruring::runtime::timer
