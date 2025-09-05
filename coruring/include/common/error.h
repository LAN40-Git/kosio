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
// ========== Fixme ==========
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

// ========== Timer Error ==========
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

// ========== Io Error ==========
class IoError : public detail::ErrorBase<IoError> {
public:
    enum Code {
        kUnknown = detail::IoErrorCodeBase,
        kWriteZero,
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
            case kWriteZero:
                return "Write return zero.";
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

namespace std {
template <typename ErrorType>
class formatter<coruring::detail::ErrorBase<ErrorType>> {
public:
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it != '}') {
            throw format_error("Invalid format specifier for Error");
        }
        return it;
    }

    auto format(const coruring::detail::ErrorBase<ErrorType>& error, auto& ctx) const {
        return format_to(ctx.out(), "{} (error {})", error.message(), error.value());
    }
};
} // namespace std