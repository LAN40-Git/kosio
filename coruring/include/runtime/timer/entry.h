#pragma once
#include "common/error.h"
#include "runtime/io/io_uring.h"
#include "io/base/callback.h"
#include <optional>

namespace coruring::runtime::timer {
namespace detail {

class TimerShared {
public:
    explicit TimerShared(uint64_t when, uint64_t register_when) noexcept
        : when_(when), register_when_(register_when) {}

public:
    auto when() const noexcept -> uint64_t { return when_; }
    void set_expiration(uint64_t expiration) noexcept { when_ = expiration; }
    auto register_when() const noexcept -> uint64_t { return register_when_; }
    void set_register_when(uint64_t register_when) noexcept { register_when_ = register_when; }


private:
    uint64_t when_;
    uint64_t register_when_;
};

class TimerHandle {
public:
    explicit TimerHandle(TimerShared inner);

private:
    TimerShared inner_;
};
} // namespace detail

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
