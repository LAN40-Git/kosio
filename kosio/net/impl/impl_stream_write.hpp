#pragma once
#include "kosio/io/io.hpp"
#include <span>

namespace kosio::net::detail {
template <class T>
struct ImplStreamWrite {
    [[REMEMBER_CO_AWAIT]]
    auto write(std::span<const char> buf) {
        return io::detail::Send{static_cast<T*>(this)->fd(), buf.data(), buf.size(), 0};
    }

    [[REMEMBER_CO_AWAIT]]
    auto write_all(std::span<const char> buf) -> async::Task<Result<void, IoError>> {
        Result<std::size_t, IoError> ret{0uz};
        while (!buf.empty()) {
            ret = co_await write(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                co_return std::unexpected{make_error<IoError>(IoError::kWriteZero)};
            }
            buf = buf.subspan(ret.value(), buf.size() - ret.value());
        }
        co_return Result<void, IoError>{};
    }
};
} // namespace kosio::net::detail