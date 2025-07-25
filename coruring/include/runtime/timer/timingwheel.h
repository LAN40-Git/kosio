#pragma once
#include <array>
#include <bitset>
#include <list>
#include "runtime/config.h"
#include "common/util/time.h"
#include "common/error.h"
#include "entry.h"

namespace coruring::runtime::detail
{
// 线程不安全
template <std::size_t MAX_LEVEL, std::size_t SLOT_SIZE>
    requires (MAX_LEVEL > 0) && (SLOT_SIZE > 0) && ((SLOT_SIZE & (SLOT_SIZE - 1)) == 0) // SLOT_SIZE必须大于0且为2的幂次
class TimingWheel : util::Noncopyable {
    static constexpr auto make_precision() {
        std::array<std::size_t, MAX_LEVEL> precision{};
        precision[0] = 64;
        for (size_t i = 1; i < MAX_LEVEL; ++i) {
            precision[i] = precision[i-1] * SLOT_SIZE;
        }
        return precision;
    }

public:
    static constexpr auto PRECISION = make_precision(); // 每层时间轮的时间范围
    static constexpr std::size_t MASK = SLOT_SIZE - 1;
    static constexpr size_t SHIFT = std::countr_zero(SLOT_SIZE);

public:
    using ENTRIES = std::list<std::unique_ptr<Entry>>;
    using BITMAP = std::bitset<SLOT_SIZE>;

    TimingWheel() = default;
    ~TimingWheel() = default;

public:
    auto add_entry(std::unique_ptr<Entry> &&entry, int64_t remaining_ms) -> std::expected<Entry*, std::error_code> {
        // 确认层级和槽位
        size_t level = 0;
        while (level + 1 < MAX_LEVEL && remaining_ms >= PRECISION[level]) {
            ++level;
        }
        size_t slot = (((remaining_ms-1) >> (level * SHIFT)) + current_slots_[level]) & MASK;

        // 将事件放入对应位置
        Entry* ret_entry = entry.get();
        wheels_[level][slot].push_back(std::move(entry));
        bitmaps_[level].set(slot);
        entries_++;
        return ret_entry;
    }

    // 底层步进
    void tick() {
        if (entries_ == 0) {
            return;
        }
        int64_t new_now_ms = util::current_ms();
        if (new_now_ms - now_ms < Config::TICK) {
            return;
        }
        now_ms = new_now_ms;
        // 1. 若当前为最后槽位，则上层步进（上层下放任务以避免对齐误差）
        if (current_slots_[0] == SLOT_SIZE - 1) {
            tick(1);
        }
        // 2. 处理槽位中的事件
        auto& entries = wheels_[0][current_slots_[0]];
        while (!entries.empty()) {
            auto entry = std::move(entries.front());
            entry->execute();
            entries.pop_front();
            entries_--;
        }
        bitmaps_[0].reset(current_slots_[0]);
        // 3. 移动到下一槽位
        current_slots_[0] = (current_slots_[0] + 1) & MASK;
    }

    void clear() noexcept {
        for (size_t level = 0; level < MAX_LEVEL; ++level) {
            for (size_t slot = 0; slot < SLOT_SIZE; ++slot) {
                if (!bitmaps_[level].test(slot)) continue;  // 位图跳过空槽
                wheels_[level][slot].clear();
                bitmaps_[level].reset(slot);
            }
            current_slots_[level] = 0;
        }
        now_ms = util::current_ms();
    }

    auto is_idle() const noexcept -> bool { return entries_ == 0; }

private:
    // 上层步进
    void tick(std::size_t level) {
        if (level >= MAX_LEVEL) [[unlikely]] {
            return;
        }
        // 1. 检查上层是否需要步进
        if (current_slots_[level] == SLOT_SIZE - 1) {
            tick(level + 1);
        }
        // 2. 下放当前槽位事件
        auto& entries = wheels_[level][current_slots_[level]];
        while (!entries.empty()) {
            auto entry = std::move(entries.front());
            entries.pop_front();
            int64_t remaining_ms = entry->expiration_ms_ - now_ms;
            if (remaining_ms <= 0) [[unlikely]] {
                entry->execute();
                entries_--;
                continue;
            }
            // 确认层级和槽位
            size_t to_level = 0;
            while (to_level + 1 < MAX_LEVEL && remaining_ms >= PRECISION[to_level]) {
                ++to_level;
            }
            size_t slot = (((remaining_ms-1) >> (to_level * SHIFT)) + current_slots_[to_level]) & MASK;

            // 将事件放入对应位置
            wheels_[to_level][slot].push_back(std::move(entry));
        }
        bitmaps_[level].reset(current_slots_[level]);
        // 3. 步进
        current_slots_[level] = (current_slots_[level] + 1) & MASK;
     }

private:
    // wheels_[层级][槽位] -> 超时事件
    std::array<std::array<ENTRIES, SLOT_SIZE>, MAX_LEVEL> wheels_;
    // current_slots_[层级][当前槽位] -> 当前层级所在的槽位
    std::array<std::size_t, MAX_LEVEL> current_slots_{0};
    // 位图，标记槽位是否为空
    std::array<BITMAP, MAX_LEVEL> bitmaps_{0};
    // 当前时间缓存
    int64_t now_ms{util::current_ms()};
    uint64_t entries_{0};
};
} // namespace coruring::runtime::detail
