#pragma once
#include <chrono>
#include <coroutine>
#include "io/base/callback.h"

namespace coruring::runtime::detail
{
class Entry {
public:
    Entry(std::chrono::steady_clock::time_point expiration_time, std::coroutine_handle<> handle)
        : expiration_time_{expiration_time}, handle_{handle} {}

    Entry(std::chrono::steady_clock::time_point expiration_time, io::detail::Callback *data)
        : expiration_time_{expiration_time}, data_{data} {}
    
private:
    std::chrono::steady_clock::time_point expiration_time_{};
    std::coroutine_handle<>               handle_{nullptr};
    io::detail::Callback                 *data_{nullptr};
    
};
}
