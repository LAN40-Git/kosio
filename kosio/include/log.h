#pragma once
#include "log/logger.h"
#include "common/util/singleton.h"

namespace kosio::log {
inline auto& console = util::Singleton<ConsoleLogger>::instance();
using detail::LogLevel;
} // namespace kosio::log
