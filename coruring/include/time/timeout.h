#pragma once

namespace coruring::io::detail {
    template <class T>
    class IoRegistrator;
}

namespace coruring::time
{
namespace detail
{
    template <typename T>
        requires std::derived_from<T, io::detail::IoRegistrator<T>>
    class Timeout : public T {
    public:
        Timeout(T&& io)
            : T{std::move(io)} {}

    public:

    };
}
} // namespace coruring::timer