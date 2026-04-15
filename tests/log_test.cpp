#include "kosio/log.hpp"
#include "kosio/common/debug.hpp"

auto main() -> int {
    SET_LOG_LEVEL(kosio::log::LogLevel::Info);
    int times{1000000};
    auto start_ms = kosio::util::current_ms();
    while (times > 0) {
        --times;
        LOG_INFO("this is a test");
    }
    auto end_ms = kosio::util::current_ms();
    LOG_INFO("{}", end_ms - start_ms);
}
