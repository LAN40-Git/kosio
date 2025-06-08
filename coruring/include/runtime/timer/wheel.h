#pragma once
#include <cstddef>
#include <list>

#include "entry.h"
#include "runtime/config.h"

namespace coruring::runtime::detail
{
class HierarchicalTimingWheel {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;
    using SlotContainer = std::vector<std::list<Entry>>;

    // 最大时间轮级数
    constexpr static std::size_t LEVELS{6uz};
    // 时间轮每级槽位
    constexpr static std::size_t SLOT_SIZE{64uz};
    // 掩码
    constexpr static std::size_t MASK{SLOT_SIZE-1};
    // 每层精度
    constexpr static std::size_t PRECISION[LEVELS]{
        1,
        64,
        64 * 64,
        64 * 64 * 64,
        64 * 64 * 64 * 64,
        64 * 64 * 64 * 64 * 64};

    std::array<SlotContainer, LEVELS> slots_;
};
}
