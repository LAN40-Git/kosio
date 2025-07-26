#pragma once
#include "file.h"
#include "level.h"
#include "buffer.h"
#include <list>
#include <thread>
#include <string>
#include <iostream>
#include <atomic>
#include <condition_variable>
#include <source_location>

namespace coruring::log {
namespace detail {
class FmtWithSourceLocation {
public:
    template <typename T>
        requires std::constructible_from<std::string_view, T>
    FmtWithSourceLocation(T&& fmt, std::source_location sl = std::source_location::current())
        : fmt_(std::forward<T>(fmt))
        , sl_(sl) {}

    [[nodiscard]]
    constexpr auto fmt() const -> std::string_view {
        return fmt_;
    }

    [[nodiscard]]
    constexpr auto source_location() const -> const std::source_location& {
        return sl_;
    }

private:
    std::string_view     fmt_;
    std::source_location sl_;
};

struct LogRecord {
    const char *datetime;
    const char *filename;
    size_t      line;
    std::string log;
};

template <class LoggerType>
class BaseLogger
{
public:
    BaseLogger() = default;
    ~BaseLogger() noexcept = default;

    BaseLogger(const BaseLogger&) = delete;
    BaseLogger& operator=(const BaseLogger&) = delete;

public:
    void set_level(LogLevel level) noexcept {
        level_ = level;
    }

    [[nodiscard]]
    auto level() const noexcept -> LogLevel {
        return level_;
    }

    template <typename... Args>
    void debug(FmtWithSourceLocation fmt, Args&& ...args) {
        format<LogLevel::Debug>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(FmtWithSourceLocation fmt, Args&& ...args) {
        format<LogLevel::Info>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(FmtWithSourceLocation fmt, Args&& ...args) {
        format<LogLevel::Warn>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(FmtWithSourceLocation fmt, Args&& ...args) {
        format<LogLevel::Error>(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void fatal(FmtWithSourceLocation fmt, Args&& ...args) {
        format<LogLevel::Fatal>(fmt, std::forward<Args>(args)...);
    }

private:
    template <LogLevel LEVEL, typename... Args>
    void format(const FmtWithSourceLocation& fwsl, const Args& ...args) {
        thread_local std::array<char, 64> buffer_{};
        thread_local time_t               last_second{0};

        if (LEVEL < level_) {
            return;
        }

        time_t current_second = ::time(nullptr);
        if (current_second != last_second) {
            tm tm_time{};
            ::localtime_r(&current_second, &tm_time);
            ::strftime(buffer_.data(), buffer_.size(), "%Y-%m-%d %H:%M:%S", &tm_time);
            last_second = current_second;
        }

        const auto &fmt = fwsl.fmt();
        const auto &sl  = fwsl.source_location();
        static_cast<LoggerType*>(this)->template log<LEVEL>(
            LogRecord{
                buffer_.data(),
                sl.file_name(),
                sl.line(),
                std::vformat(fmt, std::make_format_args(args...))
            }
        );
    }

private:
    LogLevel level_{LogLevel::Debug};
};
} // namespace detail

class ConsoleLogger : public detail::BaseLogger<ConsoleLogger> {
public:
    template <detail::LogLevel level>
    void log(const detail::LogRecord& record) {
        std::cout << std::format("{} {} {}:{} {}\n",
                                    record.datetime,
                                    level_to_string(level),
                                    record.filename,
                                    record.line,
                                    record.log);
    }
};

class FileLogger : public detail::BaseLogger<FileLogger> {
public:
    explicit FileLogger(std::string_view file_base_name)
        : file_{file_base_name}
        , current_buffer_{std::make_unique<Buffer>()} 
        , thread_{&FileLogger::work, this} {
        for (int i = 0; i < 2; ++i) {
            empty_buffers_.emplace_back(std::make_unique<Buffer>());
        }
    }

    ~FileLogger() {
        running_ = false;
        cond_.notify_one();
        if (thread_.joinable()) thread_.join();
    }

    template <detail::LogLevel level>
    void log(const detail::LogRecord& record) {
        if (!running_) [[unlikely]] {
            return;
        }
        std::string msg{std::format("{} {} {}\n",
                                    record.datetime,
                                    level_to_string(level),
                                    record.log)};

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (current_buffer_->writable_bytes() > msg.size()) {
                current_buffer_->write(msg);
                return;
            }
            full_buffers_.push_back(std::move(current_buffer_));
            if (!empty_buffers_.empty()) {
                current_buffer_ = std::move(empty_buffers_.front());
                empty_buffers_.pop_front();
            } else {
                current_buffer_ = std::make_unique<Buffer>();
            }
            current_buffer_->write(msg);
        }
        cond_.notify_one();
    }

    void set_max_file_size(off_t roll_size) noexcept {
        file_.set_max_file_size(roll_size);
    }

private:
    void work() {
        constexpr std::size_t max_buffer_list_size = 15;

        while (running_) {
            {
                std::unique_lock lock(mutex_);
                cond_.wait_for(lock, std::chrono::milliseconds(3), [this]() -> bool {
                    return !this->full_buffers_.empty();
                });
                // 交换缓冲区
                full_buffers_.push_back(std::move(current_buffer_));
                if (!empty_buffers_.empty()) {
                    current_buffer_ = std::move(empty_buffers_.front());
                    empty_buffers_.pop_front();
                } else {
                    current_buffer_ = std::make_unique<Buffer>();
                }
            }

            // 若缓冲区数量超过最大值，则仅保留最早的双缓区
            if (full_buffers_.size() > max_buffer_list_size) {
                std::cerr << std::format("Dropped log messages {} larger buffers\n", full_buffers_.size() - 2);
                full_buffers_.resize(2);
            }

            for (auto &buffer : full_buffers_) {
                file_.write(buffer->data(), buffer->size());
                buffer->reset();
            }

            if (full_buffers_.size() > 2) {
                full_buffers_.resize(2);
            }
            file_.flush();
            empty_buffers_.splice(empty_buffers_.end(), full_buffers_);
        }

        if (!current_buffer_->empty()) {
            full_buffers_.emplace_back(std::move(current_buffer_));
        }
        for (auto& buffer : full_buffers_) {
            file_.write(buffer->data(), buffer->size());
        }
        file_.flush();
    }

private:
    using Buffer = detail::LogBuffer<4000 * 1024>; // 4MB
    using BufferPtr = std::unique_ptr<Buffer>;

    detail::LogFile         file_;
    BufferPtr               current_buffer_{};
    std::list<BufferPtr>    empty_buffers_{};
    std::list<BufferPtr>    full_buffers_{};
    std::mutex              mutex_{};
    std::condition_variable cond_{};
    std::thread             thread_{};
    std::atomic<bool>       running_{true};
};
} // namespace coruring::log