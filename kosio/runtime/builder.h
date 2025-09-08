#pragma once
#include "kosio/runtime/config.h"
#include "kosio/runtime/scheduler/current_thread/handle.h"
#include "kosio/runtime/scheduler/multi_thread/handle.h"
#include <string>
#include <thread>

namespace kosio::runtime {
namespace detail {
template <typename H>
class Runtime;

template <typename B, typename H>
class Builder {
private:
    Builder() = default;

public:
    [[nodiscard]]
    auto entries(std::size_t entries) -> Builder& {
        config_.entries = entries;
        return *this;
    }

    [[nodiscard]]
    auto submit_interval(std::size_t submit_interval) -> Builder& {
        config_.submit_interval = submit_interval;
        return *this;
    }

    [[nodiscard]]
    auto io_interval(std::size_t io_interval) -> Builder& {
        config_.io_interval = io_interval;
        return *this;
    }

    [[nodiscard]]
    auto global_queue_interval(std::size_t global_queue_interval) -> Builder& {
        config_.global_queue_interval = global_queue_interval;
        return *this;
    }

    [[nodiscard]]
    auto num_workers(std::size_t num_workers) -> Builder& {
        config_.num_workers = num_workers;
        return *this;
    }

    /// Return an established runtime
    [[nodiscard]]
    auto build() -> Runtime<H> {
        return Runtime<H>{std::move(config_)};
    }

public:
    [[nodiscard]]
    static auto options() -> B {
        return B{};
    }

    [[nodiscard]]
    static auto default_create() {
        return B{}.build();
    }

protected:
    Config config_;
};
}

class CurrentThreadBuilder : public detail::Builder<CurrentThreadBuilder, scheduler::current_thread::Handle> {
};

class MultiThreadBuilder : public detail::Builder<MultiThreadBuilder, scheduler::multi_thread::Handle> {
public:
    [[nodiscard]]
    auto set_num_workers(std::size_t num_worker_threads) -> MultiThreadBuilder& {
        config_.num_workers = num_worker_threads;
        return *this;
    }
};

} // namespace kosio::runtime
