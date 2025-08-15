#include "runtime/timer/timer.h"

coruring::runtime::timer::Timer::Timer()
    : start_time_(util::current_ms())
    , elapsed_(0) {
    for (auto level = 0; level < runtime::detail::NUM_LEVELS; ++level) {
        levels_[level] = std::make_unique<detail::Level>(level);
    }
    t_timer = this;
}

auto coruring::runtime::timer::Timer::insert(coruring::io::detail::Callback *data, uint64_t expiration_time)
const noexcept -> Result<Entry *, TimerError> {
    if (expiration_time <= start_time_ + elapsed_) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kPassedTime)};
    }
    auto when = expiration_time - start_time_;
    if (when > MAX_DURATION) [[unlikely]] {
        return std::unexpected{make_error<TimerError>(TimerError::kTooLongTime)};
    }
    auto entry = std::make_unique<Entry>(Entry{data, expiration_time});

    auto level = level_for(when);

    auto* entry_ptr = entry.get();
    levels_[level]->add_entry(std::move(entry), when);
    return entry_ptr;
}

void coruring::runtime::timer::Timer::remove(Entry *entry) noexcept {
    // 若事件未被分层时间轮取消，则设置 data_ 为 nullptr
    // 告诉分层时间轮此事件已经完成，不需要再取消
    if (entry->data_) {
        entry->data_ = nullptr;
    }
}

auto coruring::runtime::timer::Timer::next_expiration()
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

void coruring::runtime::timer::Timer::handle_expired_entries(uint64_t now) {
    while (true) {
        // 首先处理到期事件
        handle_pending_entries();

        // 获取下一个到期信息
        auto expiration = next_expiration();
        // 若到期信息中的最小到期时间 deadline（具体含义可见 level.cpp 中的注释）
        // 大于当前分层时间轮运行的时间，则说明当前已经没有到期事件了，直接退出循环即可
        if (expiration.value().deadline > now) {
            break;
        }

        // 处理到期信息
        process_expiration(expiration.value());

        // 推进分层时间轮时间
        elapsed_ = expiration.value().deadline;
    }

    // 推进分层时间轮时间
    elapsed_ = now;
}

auto coruring::runtime::timer::Timer::take_entries(const Expiration& expiration)
const noexcept -> EntryList {
    return levels_[expiration.level]->take_slot(expiration.slot);
}

void coruring::runtime::timer::Timer::process_expiration(const Expiration &expiration) {
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

        // 若事件到期，则加入 pending_
        if (entry->expiration_time_ <= start_time_ + expiration.deadline) {
            pending_.push_back(std::move(entry));
        }
    }
}

void coruring::runtime::timer::Timer::handle_pending_entries() {
    while (!pending_.empty()) {
        auto entry = std::move(pending_.front());
        pending_.pop_front();
        entry->execute();
    }
}

auto coruring::runtime::timer::Timer::level_for(uint64_t when)
    noexcept -> std::size_t {
    std::size_t level = 0;
    while (when >= LEVEL_RANGE[level]) {
        level++;
    }

    return level;
}
