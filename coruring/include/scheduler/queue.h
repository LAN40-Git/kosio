#pragma once
#include "third_party/concurrentqueue.h"

namespace coruring::scheduler::detail
{
template <typename T>
class Queue {
public:

private:
    moodycamel::ConcurrentQueue<T> queue_;
};
}
