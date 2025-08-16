#pragma once
#include <expected>
#include <string_view>
#include <cstring>
#include <format>

namespace coruring {
namespace detail {
// 错误码起始值
static inline constexpr int ErrorCodeBase = 6000;
// 错误码间隔值
static inline constexpr int ErrorCodeInterval = 1000;
// 常规错误码起始值
static inline constexpr int CommonErrorCodeBase = ErrorCodeBase;
// 定时器错误码起始值
static inline constexpr int TimerErrorCodeBase = CommonErrorCodeBase + ErrorCodeInterval;
// Io 错误码起始值
static inline constexpr int IoErrorCodeBase = TimerErrorCodeBase + ErrorCodeInterval;

template <class DeriverError>
class ErrorBase {
public:
    explicit ErrorBase(int error_code)
        : error_code_(error_code) {}

public:
    [[nodiscard]]
    auto value() const noexcept -> int { return error_code_; }

    [[nodiscard]]
    auto message() const noexcept -> std::string_view {
        return static_cast<const DeriverError*>(this)->error_message();
    }

protected:
    int error_code_;
};
} // namespace detail

/* Template of DriverError
class Fixme : public detail::ErrorBase<Fixme> {
public:
    enum Code {
        kUnknown = Fixme,
    };

public:
    explicit Fixme(int error_code)
        : ErrorBase<Fixme>(error_code) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (static_cast<Code>(error_code_)) {
            case kUnknown:
                return "Unknown Fixme error.";
            default:
                return strerror(error_code_);
        }
    }
};
*/

// ========== 常规错误 ==========
class CommonError : public detail::ErrorBase<CommonError> {
public:
    enum Code {
        kUnknown = detail::CommonErrorCodeBase,
    };

public:
    explicit CommonError(int error_code)
        : ErrorBase<CommonError>(error_code) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (static_cast<Code>(error_code_)) {
            case kUnknown:
                return "Unknown common error.";
            default:
                return strerror(error_code_);
        }
    }
};

// ========== 定时器错误 ==========
class TimerError : public detail::ErrorBase<TimerError> {
public:
    enum Code {
        kUnknown = detail::TimerErrorCodeBase,
        kPassedTime,
    };

public:
    explicit TimerError(Code error_code)
        : ErrorBase<TimerError>(static_cast<int>(error_code)) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (static_cast<Code>(error_code_)) {
            case kUnknown:
                return "Unknown timer error.";
            case kPassedTime:
                return "Time has passed.";
            default:
                return strerror(error_code_);
        }
    }
};

// ========== 输入输出错误 ==========
class IoError : public detail::ErrorBase<IoError> {
public:
    enum Code {
        kUnknown = detail::IoErrorCodeBase,
    };
public:
    explicit IoError(Code error_code)
        : ErrorBase<IoError>(static_cast<int>(error_code)) {}

public:
    [[nodiscard]]
    auto error_message() const noexcept -> std::string_view {
        switch (static_cast<Code>(error_code_)) {
            case kUnknown:
                return "Unknown io error.";
            default:
                return strerror(error_code_);
        }
    }
};

template <class ErrorType>
    requires std::derived_from<ErrorType, detail::ErrorBase<ErrorType>>
[[nodiscard]]
static inline auto make_error(int error_code) -> detail::ErrorBase<ErrorType> {
    return detail::ErrorBase<ErrorType>(error_code);
}

template <typename ResultType, class ErrorType>
using Result = std::expected<ResultType, detail::ErrorBase<ErrorType>>;
} // namespace coruring