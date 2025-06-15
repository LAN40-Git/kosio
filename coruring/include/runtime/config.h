#pragma once
#include <mutex>
#include <fstream>
#include <nlohmann/json.hpp>
#include "common/util/nocopyable.h"

namespace coruring::runtime::detail {
class Config : public util::Noncopyable {
    static constexpr std::string CONFIG_PATH = "config.json";
public:
    // ====== 默认配置 ======
    // ====== io ======
    static constexpr std::size_t ENTRIES = 2048;
    static constexpr std::size_t SUBMIT_INTERVAL = 64;
    // ====== timer ======
    static constexpr std::size_t MAX_LEVEL = 6;
    static constexpr std::size_t SLOTS = 64;
    static constexpr std::size_t TICK = 1;
    // ====== worker ======
    // 工作线程最大批量恢复的协程数量
    static constexpr std::size_t IO_BATCH_SIZE = 64;
    // 工作线程最大批量收割的IO请求数量
    static constexpr std::size_t PEEK_BATCH_SIZE = 256;
    // 窃取区间系数
    static constexpr float STEAL_FACTOR = 0.5;

    // ====== 成员变量 ======
    std::size_t entries{ENTRIES};
    std::size_t submit_interval{SUBMIT_INTERVAL};

    // 从文件加载配置 TODO：添加错误处理
    static const Config& load() {
        static Config instance;
        static std::once_flag flag;
        std::call_once(flag, [&] {
            std::ifstream file(CONFIG_PATH);
            if (file.good()) {
                nlohmann::json j;
                file >> j;
                instance.entries = j.value("entries", instance.entries);
                instance.submit_interval = j.value("submit_interval", instance.submit_interval);
            } else {
                save(instance);
            }
        });
        return instance;
    }

private:
    Config() = default;

private:
    // 保存配置到文件
    static void save(const Config& config) {
        nlohmann::json j;
        j["entries"] = config.entries;
        j["submit_interval"] = config.submit_interval;

        std::ofstream file(CONFIG_PATH);
        if (!file.good()) {
            throw std::runtime_error("Failed to open config file");
        }
        file << j.dump(4);
    }
};
} // namespace coruring::runtime::detail