#pragma once
#include "io/impl/impl_async_recv.h"

namespace coruring::socket::detail
{
template <class T>
struct ImplStreamRead : public io::detail::ImplAsyncRecv<T> {
    [[REMEMBER_CO_AWAIT]]
    auto peek(std::span<char> buf) const noexcept {
        return io::detail::Recv(static_cast<const T*>(this)->fd(),
                                buf.data(),
                                buf.size_bytes(),
                                MSG_PEEK);
    }
};
}
