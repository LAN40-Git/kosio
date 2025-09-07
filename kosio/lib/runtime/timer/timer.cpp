#include "kosio/include/runtime/timer/timer.h"
#include "kosio/include/common/debug.h"

kosio::runtime::timer::Timer::Timer()
    : start_time_(util::current_ms())
    , elapsed_(0) {
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        levels_[level] = std::make_unique<detail::Level>(level);
    }
    t_timer = this;
}

auto kosio::runtime::timer::Timer::insert(kosio::io::detail::Callback *data, uint64_t expiration_time)
const noexcept -> Result<Entry *, TimerError> {
    if (expiration_time <= start_time_ + elapsed_) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    // 事件到期的时间
    auto when = expiration_time - start_time_;
    auto [entry, result] = Entry::make(data, expiration_time);

    auto level = level_for(when);
    levels_[level]->add_entry(std::move(entry), when);
    return result;
}

auto kosio::runtime::timer::Timer::insert(std::coroutine_handle<> handle, uint64_t expiration_time)
const noexcept -> Result<Entry *, TimerError> {
    if (expiration_time <= start_time_ + elapsed_) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    // 事件到期的时间
    auto when = expiration_time - start_time_;
    auto [entry, result] = Entry::make(handle, expiration_time);

    auto level = level_for(when);
    levels_[level]->add_entry(std::move(entry), when);
    return result;
}

void kosio::runtime::timer::Timer::remove(Entry *entry) noexcept {
    entry->data_ = nullptr;
    entry->handle_ = nullptr;
}

auto kosio::runtime::timer::Timer::next_expiration()
const noexcept -> std::optional<Expiration> {
    // 否则从第 0 层开始遍历获取到 最近一个非空槽位信息所在位置
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        if (auto expiration = levels_[level]->next_expiration(elapsed_)) {
            return expiration;
        }
    }
    // 返回空表示可以无限睡眠直到被唤醒
    return std::nullopt;
}

auto kosio::runtime::timer::Timer::next_expiration_time() const noexcept -> std::optional<uint64_t> {
    if (auto expiration = next_expiration()) {
        return expiration->deadline - elapsed_;
    } else {
        return std::nullopt;
    }
}

auto kosio::runtime::timer::Timer::start_time() const noexcept -> uint64_t {
    return start_time_;
}

auto kosio::runtime::timer::Timer::elapsed() const noexcept -> uint64_t {
    return elapsed_;
}

auto kosio::runtime::timer::Timer::take_entries(const Expiration& expiration)
const noexcept -> EntryList {
    return levels_[expiration.level]->take_slot(expiration.slot);
}

void kosio::runtime::timer::Timer::process_expiration(const Expiration &expiration) noexcept {
    auto entries = take_entries(expiration);

    // 若槽位位于第 0 层，说明此槽位中的所有时间都到期
    // 直接放入 pending_ 中等待立即处理
    if (expiration.level == 0) {
        pending_.splice(pending_.end(), entries);
        return;
    }

    // 若槽位不在第 0 层，则说明此槽位中可能有未到期的事件
    // 需要遍历处理
    while (!entries.empty()) {
        auto entry = std::move(entries.front());
        entries.pop_front();

        auto when = entry->expiration_time_ - start_time_;

        if (when <= expiration.deadline) {
            // 若事件到期，则加入 pending_
            pending_.push_back(std::move(entry));
        } else {
            // 若事件未到期，则以 expiration.deadline 为
            // 分层时间轮当前时间，重新计算事件所在层级并插入
            auto level = level_for(expiration.deadline, when);
            levels_[level]->add_entry(std::move(entry), when);
        }
    }
}

auto kosio::runtime::timer::Timer::level_for(uint64_t when)
const noexcept -> std::size_t {
    return level_for(elapsed_, when);
}

auto kosio::runtime::timer::Timer::level_for(uint64_t elapsed, uint64_t when)
    noexcept -> std::size_t {
    constexpr uint64_t SLOT_MASK = (1ULL << 6) - 1;

    // 使用异或计算出差值，同时将低 6 位置 1 以使 when - elapsed < 64
    // 的定时器事件能够进入最底层，masked 对应的是事件剩余的时间
    auto masked = elapsed ^ when | SLOT_MASK;
    if (masked >= MAX_DURATION) {
        // TODO: 理解 tokio 中对 MAX_DURATION - 1 的作用
        masked = MAX_DURATION - 1;
    }

    // 从最高位开始计算 0 的个数 leading_zeros
    // 计算此值实际是为了计算 masked 的最高非零位所在位数
    auto leading_zeros = std::countl_zero(masked);
    // significant 对应的层级
    // 0-5:  0 层
    // 6-10: 1 层
    // ...
    // 可以发现每 6 位对应一个层级
    // 由于从下标 0 开始计算，因此用 63 - leading_zeros
    // 计算出事件剩余事件所在的层级范围
    auto significant = 63 - leading_zeros;
    // 计算确切的层级数
    return significant / runtime::detail::NUM_LEVELS;
}
