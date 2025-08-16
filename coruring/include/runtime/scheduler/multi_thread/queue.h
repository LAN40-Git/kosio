#pragma once
#include "third_party/concurrentqueue.h"
#include "runtime/config.h"
#include <coroutine>

namespace coruring::runtime::scheduler::multi_thread {
namespace detail {
class BaseQueue {
public:
    [[nodiscard]]
    auto enqueue(std::coroutine_handle<>&& task) -> bool {
        return tasks_.enqueue(std::move(task));
    }

    template <typename It>
    [[nodiscard]]
    auto enqueue_bulk(It itemFirst, std::size_t count) -> bool {
        return tasks_.enqueue_bulk(itemFirst, count);
    }

    [[nodiscard]]
    auto try_dequeue(std::coroutine_handle<>& task) -> bool {
        return tasks_.try_dequeue(task);
    }

    template <typename  It>
    [[nodiscard]]
    auto try_dequeue_bulk(It itemFirst, std::size_t max)
    -> std::size_t {
        return tasks_.try_dequeue_bulk(itemFirst, max);
    }

    [[nodiscard]]
    auto size_approx() const -> std::size_t {
        return tasks_.size_approx();
    }

    [[nodiscard]]
    auto empty() const -> bool {
        return tasks_.size_approx() == 0;
    }

private:
    moodycamel::ConcurrentQueue<std::coroutine_handle<>> tasks_;
};
} // namespace detail

class LocalQueue : public detail::BaseQueue {
public:
    [[nodiscard]]
    auto steal_into(LocalQueue& dst, std::size_t count) -> bool {
        static std::array<std::coroutine_handle<>,
        runtime::detail::IO_INTERVAL> buf = {};
        auto steal_count = try_dequeue_bulk(buf.data(), count);
        return dst.enqueue_bulk(buf.data(), steal_count);
    }
};

class GlobalQueue : public detail::BaseQueue {};
} // namespace coruring::runtime::scheduler::multi_thread
