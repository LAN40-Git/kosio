#pragma once
#include <expected>
#include <string_view>
#include <cassert>
#include <cstring>
#include <format>

namespace coruring {
namespace detail {
// 错误码起始值
static inline constexpr int ErrorCodeBase = 6000;
// 错误码间隔值
static inline constexpr int ErrorCodeInterval = 1000;
// 定时器错误码起始值
static inline constexpr int TimerErrorCodeBase = ErrorCodeBase + ErrorCodeInterval;
// Io 错误码起始值
static inline constexpr int IoErrorCodeBase = TimerErrorCodeBase + ErrorCodeInterval;

template <class ErrorType>
class ErrorBase {
public:
    explicit ErrorBase(int error_code)
        : error_code_(error_code) {}

public:
    [[nodiscard]]
    auto value() const noexcept -> int { return error_code_; }

    [[nodiscard]]
    auto message() const noexcept -> std::string_view {
        return static_cast<ErrorType*>(this)->error_message(error_code_);
    }

protected:
    int error_code_;
};
} // namespace detail

enum class TimerErrorCode : int {
    PassedTime = detail::TimerErrorCodeBase,
};

enum class IoErrorCode : int {
    Example = detail::IoErrorCodeBase,
};

namespace detail {
class TimerError : public ErrorBase<TimerError> {
public:
    explicit TimerError(TimerErrorCode error_code)
        : ErrorBase<TimerError>(static_cast<int>(error_code)) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (error_code_) {
            case TimerErrorCode::PassedTime:
                return "Time has passed";
            default:
                return strerror(error_code_);
        }
    }
};

class IoError : public ErrorBase<IoError> {
public:
    explicit IoError(IoErrorCode error_code)
        : ErrorBase<IoError>(static_cast<int>(error_code)) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (error_code_) {
            case IoErrorCode::Example:
                return "This is a example";
            default:
                return strerror(error_code_);
        }
    }
};
} // namespace detail

static inline auto make_timer_error(TimerErrorCode error_code) -> detail::TimerError {
    return detail::TimerError(error_code);
}

static inline auto make_io_error(IoErrorCode error_code) -> detail::IoError {
    return detail::IoError(error_code);
}

template <class T, class ErrorType>
using Result = std::expected<T, detail::ErrorBase<ErrorType>>;
} // namespace coruring
