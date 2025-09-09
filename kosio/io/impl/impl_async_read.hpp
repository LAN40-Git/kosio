#pragma once
#include "kosio/io/io.hpp"

namespace kosio::io::detail {
template<class T>
struct ImplAsyncRead {
    [[REMEMBER_CO_AWAIT]]
    auto read(std::span<char> buf) const noexcept {
        return Read{static_cast<const T *>(this)->fd(),
                    buf.data(),
                    static_cast<unsigned int>(buf.size_bytes()),
                    static_cast<std::size_t>(-1)};
    }

    [[REMEMBER_CO_AWAIT]]
    auto recv_exact(std::span<char> buf) const noexcept
    -> async::Task<Result<void, IoError>> {
        Result<std::size_t, IoError> ret{0uz};
        while (!buf.empty()) {
            ret = co_await this->read(buf);
            if (!ret) [[unlikely]] {
                co_return std::unexpected{ret.error()};
            }
            if (ret.value() == 0) {
                co_return std::unexpected{make_error<IoError>(IoError::kEarlyEOF)};
            }
            buf = buf.subspan( ret.value(), buf.size_bytes() - ret.value());
        }
        co_return Result<void, IoError>{};
    }
};
} // namespace kosio::io::detail
