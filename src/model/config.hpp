#pragma once
#include <string>
#include <map>

// 声明全局配置 Map
extern std::map<std::string, std::string> g_config;

// 一些好用的工具函数声明
namespace config {
    // 写入或覆盖配置
    void set(const std::string& key, const std::string& value);
    
    // 获取配置，如果不存在则返回默认值
    std::string get(const std::string& key, const std::string& default_value = "");
    
    // 检查某个键是否被“注册”（存在）
    bool has(const std::string& key);
    
    // 打印所有已注册的配置（给你的 -Op @@enable_config 用的）
    void print_all();
}