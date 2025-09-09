#pragma once
#include <chrono>
#include <coroutine>

namespace kosio::runtime::timer {
class Entry;
} // namespace kosio::runtime::timer

namespace kosio::io::detail {
struct Callback {
    std::coroutine_handle<> handle_{nullptr};
    int                     result_;
    uint64_t                deadline_;
    runtime::timer::Entry  *entry_{nullptr};
};
} // namespace kosio::io::detail