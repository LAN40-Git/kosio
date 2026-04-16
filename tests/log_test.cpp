#include "kosio/log.hpp"
#include "kosio/common/debug.hpp"

auto main() -> int {
    SET_LOG_LEVEL(kosio::log::LogLevel::Info);
    int times{100};
    auto start_time = kosio::util::format_time(kosio::util::current_ms());
    while (times > 0) {
        --times;
        LOG_INFO("this is a test");
    }
    auto end_time = kosio::util::format_time(kosio::util::current_ms());
    LOG_INFO("{}-{}", start_time, end_time);
}
