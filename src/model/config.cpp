#include "config.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

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
        for (const auto& [key, value] : g_config) {
            std::cerr << key << " " << value << "\n";
        }
    }

    void parse_ksv_string(const std::string& content) {
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            if (line[32]!=' ') continue;
            if (line[33]!='@') continue;
            set(line.substr(0, 32), line.substr(33));
        }
    }
}