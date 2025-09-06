#pragma once
#include "kosio/include/log/logger.h"
#include "kosio/include/common/util/singleton.h"

namespace kosio::log {
inline auto& console = util::Singleton<ConsoleLogger>::instance();
using detail::LogLevel;
} // namespace kosio::log
