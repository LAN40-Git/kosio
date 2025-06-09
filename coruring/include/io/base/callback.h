#pragma once
#include <chrono>
#include <coroutine>

namespace coruring::io::detail
{
struct Callback {
    std::coroutine_handle<> handle_{nullptr};
    int result_;
};
}