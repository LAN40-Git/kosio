#pragma once
#include <cassert>
#include <expected>
#include <string_view>
#include <cstring>
#include <format>

namespace kosio {
class Error {
public:
    enum ErrorCode {
        kUnknown = 8000,
        kPassedTime,
        kWriteZero,
        kEarlyEOF,
        kInvalidAddresses,
        kJsonParseFailed,
        kReuniteFailed,
        kEmptySPSCQueue,
    };

public:
    explicit Error(int err_code)
        : error_code_{err_code} {}

public:
    [[nodiscard]]
    auto value() const noexcept -> int {
        return error_code_;
    }

    [[nodiscard]]
    auto message() const noexcept -> std::string_view {
        switch (error_code_) {
            case kUnknown:
                return "Unknown error.";
            case kPassedTime:
                return "Time has passed.";
            case kWriteZero:
                return "Write return zero.";
            case kEarlyEOF:
                return "Read EOF too early.";
            case kInvalidAddresses:
                return "Invalid addresses.";
            case kJsonParseFailed:
                return "Failed to parse json file.";
            case kReuniteFailed:
                return "Failed to reunite tcp stream.";
            case kEmptySPSCQueue:
                return "Empty spsc queue.";
            default:
                return strerror(error_code_);
        }
    }

private:
    int error_code_;
};

[[nodiscard]]
static inline auto make_error(int error_code) -> Error {
    assert(error_code >= 0);
    return Error{error_code};
}

template <typename T>
using Result = std::expected<T, Error>;
} // namespace kosio

namespace std {
template <>
struct formatter<kosio::Error> {
public:
    constexpr auto parse(format_parse_context &context) {
        auto it{context.begin()};
        auto end{context.end()};
        if (it == end || *it == '}') {
            return it;
        }
        ++it;
        if (it != end && *it != '}') {
            throw format_error("Invalid format specifier for Error");
        }
        return it;
    }

    auto format(const kosio::Error &error, auto &context) const noexcept {
        return format_to(context.out(), "{} (error {})", error.message(), error.value());
    }
};
} // namespace std