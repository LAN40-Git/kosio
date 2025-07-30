#pragma once
#include "runtime/timer/entry.h"

namespace coruring::runtime::timer::wheel::detail {
class Level {
public:


private:
    std::size_t level_;
    // 最低有效位表示时隙零
    uint64_t occupied_{};
    // 槽位，用于存储任务队列
    timer::detail::EntryList slots_{};
};
} // namespace coruring::runtime::timer::wheel::detail
