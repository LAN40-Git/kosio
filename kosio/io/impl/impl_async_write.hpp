#pragma once
#include "kosio/io/io.hpp"

namespace kosio::io::detail {
template<class T>
struct ImplAsyncWrite {
    [[REMEMBER_CO_AWAIT]]
    auto write(std::span<const char> buf) noexcept {
        return Write{static_cast<T *>(this)->fd(),
                     buf.data(),
                     static_cast<unsigned int>(buf.size_bytes()),
                     static_cast<std::size_t>(-1)};
    }

    [[REMEMBER_CO_AWAIT]]
    auto write_all(std::span<const char> buf) noexcept
    -> async::Task<Result<void, IoError>> {
        Result<std::size_t, IoError> ret{0uz};
        while (!buf.empty()) {
            ret = co_await this->write(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                co_return std::unexpected{make_error<IoError>(IoError::kWriteZero)};
            }
            buf = buf.subspan( ret.value(), buf.size_bytes() - ret.value());
        }
        co_return Result<void, IoError>{};
    }
};
} // namespace kosio::io::detail
