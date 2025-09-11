#pragma once
#include "kosio/common/macros.hpp"
#include <memory>

namespace kosio::sync::detail {
template <typename C>
class Sender {
    using ChannelPtr = std::shared_ptr<C>;

public:
    Sender(ChannelPtr counter)
        : channel_{counter} {}

    ~Sender() {
        close();
    }

    Sender(const Sender &other)
        : channel_{other.channel_} {}

    auto operator=(const Sender &other) -> Sender & {
        channel_ = other.channel_;
        return *this;
    }

    Sender(Sender &&other) noexcept
        : channel_{std::move(other.channel_)} {}

    auto operator=(Sender &&other) noexcept -> Sender & {
        channel_ = std::move(other->channel_);
        return *this;
    }

    [[REMEMBER_CO_AWAIT]]
    auto send(C::ValueType value) {
        return channel_->send(std::move(value));
    }

    void close() {
        if (channel_) {
            channel_.reset();
        }
    }

private:
    ChannelPtr channel_;
};
} // namespace kosio::sync::detail