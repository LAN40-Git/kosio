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
    static constexpr uint32_t ENTRIES = 1024;
    static constexpr uint32_t SUBMIT_INTERVAL = 1;
    // ====== timer ======
    static constexpr uint8_t MAX_LEVEL = 6;
    static constexpr uint16_t SLOTS = 64;

    // ====== 成员变量 ======
    uint32_t entries{ENTRIES};
    uint32_t submit_interval{SUBMIT_INTERVAL};

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