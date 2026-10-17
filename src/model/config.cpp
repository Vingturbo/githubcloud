#include "config.hpp"
#include <iostream>
#include <iomanip> // 用于格式化输出

// 真正的全局定义
std::map<std::string, std::string> g_config;

namespace config {
    void set(const std::string& key, const std::string& value) {
        g_config[key] = value;
    }
    
    std::string get(const std::string& key, const std::string& default_value) {
        auto it = g_config.find(key);
        if (it != g_config.end()) {
            return it->second;
        }
        return default_value;
    }
    
    bool has(const std::string& key) {
        return g_config.find(key) != g_config.end();
    }
    
    void print_all() {
        // 按照 .ksv 格式打印
        for (const auto& [key, value] : g_config) {
            std::cout << key << " " << value << "\n";
        }
    }
}