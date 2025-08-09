#pragma once
#include <chrono>
#include <coroutine>

namespace coruring::runtime::timer {
class Entry;
} // namespace coruring::runtime::timer

namespace coruring::io::detail {
struct Callback {
    std::coroutine_handle<> handle_{nullptr};
    int                     result_;
    int64_t                 deadline_;
    runtime::timer::Entry  *entry_{nullptr};
};
} // namespace coruring::io::detail