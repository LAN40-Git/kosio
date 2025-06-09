#pragma once
#include <array>
#include <bitset>
#include <list>
#include <list>
#include "runtime/config.h"
#include "entry.h"

namespace coruring::runtime::detail
{
template <std::size_t MAX_LEVEL, std::size_t SLOT_SIZE>
    requires (MAX_LEVEL > 0) && (SLOT_SIZE > 0) && ((SLOT_SIZE & (SLOT_SIZE - 1)) == 0) // SLOT_SIZE必须大于0且为2的幂次
class TimingWheel {
    static constexpr auto make_precision() {
        std::array<size_t, MAX_LEVEL> precision{};
        precision[0] = 1;
        for (size_t i = 1; i < MAX_LEVEL; ++i) {
            precision[i] = precision[i - 1] * SLOT_SIZE;
        }
        return precision;
    }

public:
    static constexpr auto PRECISION = make_precision();
    static constexpr std::size_t MASK = SLOT_SIZE - 1;

    TimingWheel() = default;
    ~TimingWheel() = default;
    TimingWheel(const TimingWheel&) = delete;
    TimingWheel& operator=(const TimingWheel&) = delete;

public:
    void add_entry(std::unique_ptr<Entry> &&entry, uint64_t timeout_ms) {
        std::size_t level = 0;

        // 找到对应层级
        while (level + 1 < MAX_LEVEL && timeout_ms >= PRECISION[level + 1]) {
            ++level;
        }

        // 计算槽位
        std::size_t slot = timeout_ms & MASK;

        // 将事件放入对应层级对应的槽位中
        wheels_[level][slot].push_back(std::move(entry));
        // 设置位图
        bitsets_[level].set(slot);
        // 事件计数
        ++entries_[level];
    }

private:
    using SLOTS = std::array<std::list<std::unique_ptr<Entry>>, SLOT_SIZE>;
    using BITMAP = std::bitset<SLOT_SIZE>;
    // wheels_[层级][槽位] -> 超时事件
    std::array<SLOTS, MAX_LEVEL>       wheels_{0};
    // bitsets_[层级][std::countr_zero] -> 非空槽位
    std::array<BITMAP, MAX_LEVEL>      bitsets_{0};
    // current_slots_[层级][当前槽位] -> 当前层级所在的槽位
    std::array<std::size_t, MAX_LEVEL> current_slots_{0};
    // 层级的总超时事件数量
    std::array<std::size_t, MAX_LEVEL> entries_{0};
};
}
