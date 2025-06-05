#pragma once
#include <coroutine>

namespace coruring::io
{
struct Callback {
    std::coroutine_handle<> handle_{nullptr};
    int result_;
};
}
