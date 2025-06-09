#pragma once
#include <array>
#include <bitset>
#include <list>
#include <list>
#include "runtime/config.h"
#include "common/util/time.h"
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
    using ENTRIES = std::list<std::unique_ptr<Entry>>;
    using BITMAPS = std::bitset<SLOT_SIZE>;

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
        std::size_t slot = (timeout_ms >> current_slots_[level]) & (MASK >> current_slots_[level]);

        // 将事件放入对应层级对应的槽位中
        wheels_[level][slot].push_back(std::move(entry));
    }

    // 底层步进
    void tick() {
        // 1. 若当前为最后槽位，则上层步进（上层下放任务以避免对齐误差）
        if (current_slots_[0] == SLOT_SIZE - 1) {
            tick(1);
        }
        // 2. 处理槽位中的事件
        auto& entries = wheels_[0][current_slots_[0]];
        handle_expired_entries(entries);
        // 3. 移动到下一槽位
        current_slots_[0] = (current_slots_[0] + 1) & MASK;
    }

    // 上层步进
    void tick(std::size_t level) {
        if (level >= MAX_LEVEL) {
            return;
        }
        assert(level > 0);
        // 1. 下放当前槽位事件
        auto& entries = wheels_[level][current_slots_[level]];
        for (auto& entry : entries) {
            uint64_t remaining_ms = entry->expiration_ms_ - util::current_ms();
            // TODO: 计算事件槽位
        }

    }

    void handle_expired_entries(ENTRIES& entries) {

    }

private:
    // wheels_[层级][槽位] -> 超时事件
    std::array<std::array<ENTRIES, SLOT_SIZE>, MAX_LEVEL> wheels_{0};
    // current_slots_[层级][当前槽位] -> 当前层级所在的槽位
    std::array<std::size_t, MAX_LEVEL> current_slots_{0};
    // 时间轮启动运行时间
    uint64_t start_ms_{util::current_ms()};
};
}
