#pragma once
#include <array>
#include <bitset>
#include <list>
#include "runtime/config.h"
#include "entry.h"

namespace coruring::runtime::detail
{
template <std::size_t MAX_LEVEL, std::size_t SLOT_SIZE>
    requires (MAX_LEVEL > 0) && (SLOT_SIZE > 0) && ((SLOT_SIZE & (SLOT_SIZE - 1)) == 0) // SLOT_SIZE必须大于0且为2的幂次
class HierarchicalTimingWheel {
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

    HierarchicalTimingWheel() = default;
    ~HierarchicalTimingWheel() = default;
    HierarchicalTimingWheel(const HierarchicalTimingWheel&) = delete;
    HierarchicalTimingWheel& operator=(const HierarchicalTimingWheel&) = delete;

public:
    void add_entry(std::unique_ptr<Entry> &&entry, uint64_t timeout_ms) {
        std::size_t level = 0;
        uint64_t ticks = timeout_ms;

        while (level + 1 < MAX_LEVEL && ticks >= PRECISION[level + 1]) {
            ++level;
            ticks /= SLOT_SIZE;
        }

        std::size_t slot = ticks & MASK;

        wheels_[level][slot].push_back(std::move(entry));
        bitsets_[level].set(slot);
        ++entries_[level];
    }


    void handle_entries() {

    }

private:
    // wheels_[层级][槽位] -> 超时事件
    std::array<std::array<std::list<std::unique_ptr<Entry>>, SLOT_SIZE>, MAX_LEVEL> wheels_{0};
    // bitsets_[层级][std::countr_zero] -> 非空槽位
    std::array<std::bitset<SLOT_SIZE>, MAX_LEVEL> bitsets_{0};
    // current_slots_[层级][当前槽位] -> 当前层级所在的槽位
    std::array<std::size_t, MAX_LEVEL> current_slots_{0};
    // 层级的总超时事件数量
    std::array<std::size_t, MAX_LEVEL> entries_{0};
};
}
