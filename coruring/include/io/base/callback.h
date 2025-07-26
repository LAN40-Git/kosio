#pragma once
#include <chrono>
#include <coroutine>

namespace coruring::runtime::detail {
class Entry;
} // namespace coruring::runtime::detail

namespace coruring::io::detail {
struct Callback {
    std::coroutine_handle<> handle_{nullptr};
    int result_;
    int64_t deadline_;
    runtime::detail::Entry* entry_{nullptr};
};
} // namespace coruring::io::detail