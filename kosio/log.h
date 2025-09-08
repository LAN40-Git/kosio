#pragma once
#include "kosio/log/logger.h"
#include "kosio/common/util/singleton.h"

namespace kosio::log {
inline auto& console = util::Singleton<ConsoleLogger>::instance();
using detail::LogLevel;
} // namespace kosio::log
