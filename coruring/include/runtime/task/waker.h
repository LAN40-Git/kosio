#pragma once
#include "common/util/noncopyable.h"
#include "runtime/io/io_uring.h"
#include <sys/eventfd.h>
#include <unistd.h>

namespace coruring::runtime::task::waker {
class Waker : util::Noncopyable {
public:
    Waker();
    ~Waker();
    Waker(Waker&& other) noexcept;
    auto operator=(Waker&& other) noexcept -> Waker&;

public:
    void wake_up() const;
    void turn_on();

private:
    int      fd_{-1};
    uint64_t flag_{1};
};
} // namespace coruring::runtime::task::waker
