#pragma once
#include <array>
#include <bitset>
#include <list>
#include "runtime/config.h"
#include "common/util/time.h"
#include "common/error.h"
#include "entry.h"

namespace coruring::runtime::detail {
// 线程不安全
template <std::size_t MAX_LEVEL, std::size_t SLOT_SIZE>
    requires (MAX_LEVEL > 0) && (SLOT_SIZE > 0) && ((SLOT_SIZE & (SLOT_SIZE - 1)) == 0) // SLOT_SIZE必须大于0且为2的幂次
class Wheel : util::Noncopyable {
    static constexpr auto make_precision() -> std::array<std::size_t, MAX_LEVEL> {
        std::array<std::size_t, MAX_LEVEL> precision{};
        precision[0] = 64;
        for (size_t i = 1; i < MAX_LEVEL; ++i) {
            precision[i] = precision[i-1] * SLOT_SIZE;
        }
        return precision;
    }

public:
    static constexpr auto& PRECISION = make_precision(); // 每层时间轮的时间范围
    static constexpr std::size_t MASK = SLOT_SIZE - 1;
    static constexpr size_t SHIFT = std::countr_zero(SLOT_SIZE);

public:
    using ENTRIES = std::list<std::unique_ptr<Entry>>;
    using BITMAP = std::bitset<SLOT_SIZE>;

    Wheel() = default;
    ~Wheel() = default;

public:
    auto add_entry(std::unique_ptr<Entry> &&entry, int64_t remaining_ms) -> std::expected<Entry*, std::error_code> {
        // 确认层级和槽位
        std::size_t level = 0;
        while (level + 1 < MAX_LEVEL && remaining_ms >= PRECISION[level]) {
            ++level;
        }
        std::size_t slot = (((remaining_ms-1) >> (level * SHIFT)) + current_slots_[level]) & MASK;

        // 将事件放入对应位置
        Entry* ret_entry = entry.get();
        wheels_[level][slot].push_back(std::move(entry));
        if (level == 0) {
            bitmap_.set(slot);
        }
        return ret_entry;
    }

    // 处理到期
    void process_expiration() noexcept {
        // 获取 tick 间隔，即最底层分层时间轮的槽位需要推进多少
        now_ms_ = util::current_ms();
        auto tick_interval = now_ms_ - last_tick_ms_;
        uint64_t tick;
        // 推进分层时间轮，首先需要找到最底层非空的槽位，并直接将其最近的槽位推进到此
        // 若最底层无非空槽位，则尝试推进到最后一个槽位并下放上层任务（当elapsed足够大）
        // 重复直到最底层的下一个非空槽位或最后的槽位与当前槽位的距离大于elapsed
        auto current_slot_to_last_slot = SLOT_SIZE - current_slots_[0] - 1;
        auto has_next_tick = next_expiration();
        while (tick_interval > 0) {
            tick = current_slot_to_last_slot;
            if (has_next_tick) {
                tick = std::min(tick, has_next_tick.value());
            }
            if (tick <= tick_interval) {
                tick_interval -= tick;
                current_slots_[0] += tick;
                // 若为最后一个槽位，则推进上层时间轮
                this->tick(1);
                // 处理当前槽位的任务

            }

        }
    }

    auto next_expiration() -> std::optional<uint64_t> {
        if (bitmap_.none()) {
            return std::nullopt;
        }
        return std::countr_zero(bitmap_.to_ullong());
    }

private:
    // 推进分层时间轮
    void tick(std::size_t level) {
        if (current_slots_[level] == SLOT_SIZE - 1 && level < MAX_LEVEL - 1) {
            tick(level + 1);
        }

        cascade(level);
        current_slots_[level] = (current_slots_[level] + 1) & MASK;
     }

    // 处理到期事件
    void handle_expired_entries() noexcept {
        auto& entries = wheels_[0][current_slots_[0]];
        while (!entries.empty()) {
            auto entry = std::move(entries.front());
            entries.pop_front();
            entry->execute();
        }
    }

    // 下放事件
    void cascade(std::size_t level) noexcept {
        auto& entries = wheels_[level][current_slots_[level]];
        while (!entries.empty()) {
            auto entry = std::move(entries.front());
            entries.pop_front();
            // 事件到期
            if (now_ms_ <= entry->expiration_ms_) [[unlikely]] {
                entry->execute();
                continue;
            }
            // 确认层级和槽位
            size_t to_level = 0;
            auto remaining_ms = now_ms_ - entry->expiration_ms_;
            while (to_level + 1 < MAX_LEVEL && remaining_ms >= PRECISION[to_level]) {
                ++to_level;
            }
            size_t slot = (((remaining_ms-1) >> (to_level * SHIFT)) + current_slots_[to_level]) & MASK;

            // 将事件放入对应位置
            wheels_[to_level][slot].push_back(std::move(entry));
        }
    }

private:
    std::array<std::array<ENTRIES, SLOT_SIZE>, MAX_LEVEL> wheels_;
    std::array<std::size_t, MAX_LEVEL>                    current_slots_{0};
    BITMAP                                                bitmap_{0};
    uint64_t                                              last_tick_ms_{util::current_ms()};
    uint64_t                                              now_ms_{util::current_ms()};
};
} // namespace coruring::runtime::detail
