#pragma once
#include <coroutine>

namespace coruring::runtime::detail
{
class Handle {
public:
    explicit Handle(std::coroutine_handle<> handle)
        : handle_(handle) {}
    ~Handle() { if (handle_) handle_.destroy(); }

private:
    std::coroutine_handle<> handle_{nullptr};
};
}
