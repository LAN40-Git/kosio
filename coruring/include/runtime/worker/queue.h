#pragma once
#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#endif
#include "common/util/nocopyable.h"
#include "third_party/concurrentqueue.h"

namespace coruring::runtime::detail
{
template <typename T>
class Queue : public util::Noncopyable {
public:
    bool enqueue(T&& item) {
        return queue_.enqueue(std::forward<T>(item));
    }

    bool try_dequeue(T& item) {
        return queue_.try_dequeue(item);
    }

    template <typename It>
    requires std::input_iterator<It> &&
         std::convertible_to<std::iter_value_t<It>, T>
    bool enqueue_bulk(It itemFirst, std::size_t count) {
        return queue_.enqueue_bulk(itemFirst, count);
    }

    template <typename It>
    requires std::input_iterator<It> &&
         std::convertible_to<std::iter_value_t<It>, T>
    std::size_t try_dequeue_bulk(It itemFirst, std::size_t max) {
        return queue_.try_dequeue_bulk(itemFirst, max);
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return queue_.size_approx();
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return queue_.size_approx() == 0;
    }

private:
    moodycamel::ConcurrentQueue<T> queue_;
};
}
