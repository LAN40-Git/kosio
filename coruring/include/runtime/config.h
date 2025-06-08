#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace coruring::runtime::detail {

// 最大时间轮级数
constexpr static inline std::size_t MAX_WHEEL{6uz};
// 时间轮每级槽位
constexpr static inline std::size_t SLOT_SIZE{64uz};

class Config {
public:
    // ====== 默认配置 ======
    // io_uring 队列纵深
    constexpr static uint32_t DEFAULT_ENTRIES = 1024;
    // io_uring 提交间隔
    constexpr static uint32_t DEFAULT_SUBMIT_INTERVAL = 64;

    uint32_t entries{DEFAULT_ENTRIES};
    uint32_t submit_interval{DEFAULT_SUBMIT_INTERVAL};

    Config() = default;

    // 从文件加载配置
    static Config load_from_file(const std::filesystem::path& path = "config.json") {
        Config config;

        try {
            std::ifstream file(path);
            if (file.good()) {
                nlohmann::json j;
                file >> j;
                config.entries = j.value("entries", config.entries);
                config.submit_interval = j.value("submit_interval", config.submit_interval);
            } else {
                // 文件不存在，保存默认配置
                save_to_file(config, path);
            }
        } catch (...) {}

        return config;
    }

    // 保存配置到文件
    static void save_to_file(const Config& config,
                           const std::filesystem::path& path = "config.json") {
        nlohmann::json j;
        j["entries"] = config.entries;
        j["submit_interval"] = config.submit_interval;

        std::ofstream file(path);
        if (file.good()) {
            file << j.dump(4);
        }
    }
};
} // namespace coruring::io