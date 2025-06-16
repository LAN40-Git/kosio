#pragma once
#include "io/awaiter/send.h"
#include "io/impl/impl_async_send.h"

namespace coruring::socket::detail
{
template <class T>
struct ImplStreamWrite : public io::detail::ImplAsyncSend<T> {
    [[REMEMBER_CO_AWAIT]]
    auto peek(std::span<char> buf) const noexcept {
        return io::detail::Write(static_cast<const T*>(this)->fd(),
                                buf.data(),
                                buf.size_bytes(),
                                MSG_PEEK);
    }
};
}
