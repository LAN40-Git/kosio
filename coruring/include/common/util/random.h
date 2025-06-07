#pragma once

#include <random>
#include <cinttypes>

namespace coruring::util {
class FastRand {
public:
    // thread_local instance
    static FastRand& instance() {
        thread_local FastRand instance;
        return instance;
    }

    // n > 0
    uint32_t fastrand_n(uint32_t n) {
        uint32_t threshold = (-n) % n;
        while (true) {
            uint32_t r = fastrand();
            if (r >= threshold)
                return r % n;
        }
    }

    int32_t rand_range(int32_t min, int32_t max) {
        return min + static_cast<int32_t>(fastrand_n(max - min + 1));
    }

private:
    FastRand() {
        std::random_device dev;
        one_ = static_cast<uint32_t>(dev());
        two_ = static_cast<uint32_t>(dev());
    }

    uint32_t fastrand() {
        auto s1 = one_;
        auto s0 = two_;
        s1 ^= s1 << 17;
        s1 = s1 ^ s0 ^ s1 >> 7 ^ s0 >> 16;
        one_ = s0;
        two_ = s1;
        return s0 + s1;
    }

    uint32_t one_;
    uint32_t two_;
};
} // namespace coruring::util
