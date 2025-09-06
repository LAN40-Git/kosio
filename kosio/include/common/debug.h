#pragma once

#include <cassert>
#ifdef NEED_kosio_LOG

#include "log.h"

inline auto &debug_logger = kosio::log::console;

#define SET_LOG_LEVEL(level) debug_logger.set_level(level)

#define LOG_VERBOSE(...) debug_logger.verbose(__VA_ARGS__)
#define LOG_INFO(...) debug_logger.info(__VA_ARGS__)
#define LOG_DEBUG(...) debug_logger.debug(__VA_ARGS__)
#define LOG_WARN(...) debug_logger.warn(__VA_ARGS__)
#define LOG_ERROR(...) debug_logger.error(__VA_ARGS__)
#define LOG_FATAL(...) debug_logger.fatal(__VA_ARGS__)

#else

#define SET_LOG_LEVEL(level)

#define LOG_VERBOSE(...)
#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)

#endif
