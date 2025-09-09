#pragma once
#include "kosio/log/logger.hpp"
#include "kosio/common/util/singleton.hpp"

namespace kosio::log {
inline auto& console = util::Singleton<ConsoleLogger>::instance();
using detail::LogLevel;
} // namespace kosio::log
