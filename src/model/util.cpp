#include "util.hpp"
#include "config.hpp"
#include "cds.hpp"
#include "../lib/monocypher/monocypher.h"
#include <string>
#include <map>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <fstream>

// 如果 Windows 下需要
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace util
{
    std::filesystem::path user_config_path;
    std::filesystem::path place_local_index;
    std::filesystem::path place_config_path;

    std::string user_config_content;
    std::string place_local_index_content;
    std::string place_config_content;

    cds::Root place_cds_node = cds::create();

    std::map<std::string, std::string> bang_cache;
    std::string exec_command(const std::string &cmd)
    {
        std::string result;
        char buffer[128];

        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            return "";

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }
        pclose(pipe);

        // 抹掉末尾的换行符
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        {
            result.pop_back();
        }
        return result;
    }

    // 核心：解析参数值
    std::string resolve_value(const std::string &raw_val)
    {
        // 1. 如果长度小于2，或者不以 @ 开头，说明是不合规的垃圾
        if (raw_val.length() < 2 || raw_val[0] != '@')
        {
            return "";
        }

        // 2. 判断前缀
        if (raw_val[1] == '@')
        {
            // @@ 字面量，直接切掉前两个字符
            return raw_val.substr(2);
        }
        else
        {
            if (config::get("91de27d37ac04464d4f8fd53afd0c2bc") != "@@yes")
            {
                util::message_out("8a94c5bae8543a47fb0b5dcdc86240e4");
                return "";
            }
            if (raw_val[1] == '_')
            {
                // @_ 每次执行
                return exec_command(raw_val.substr(2));
            }
            else if (raw_val[1] == '!')
            {
                std::string cmd = raw_val.substr(2);

                if (bang_cache.count(cmd) == 0)
                {
                    bang_cache[cmd] = exec_command(cmd);
                }
                return bang_cache[cmd];
            }
        }

        // 3. 如果写了单 @ 之类的奇怪东西，原样返回，让它去后面报错
        return "";
    }

    void message_out(const std::string &message_id)
    {
        std::cerr << util::resolve_value(config::get(message_id)) << std::endl;
    }

    void load_online_config(const std::string &online_config_URL) {}

    void debug_log(const std::string &message)
    {
        if (util::resolve_value(config::get("701998cfd3d1ff0378375a9826fd9edd")) == "yes")
        {
            // 1. 获取当前时间点
            auto now = std::chrono::system_clock::now();

            // 2. 获取毫秒部分
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

            // 3. 获取秒级时间并转为 UTC 的 tm 结构体 (因为末尾是 Z，必须用 gmtime)
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::gmtime(&t);

            // 4. 格式化输出
            char buf[32];
            // 先格式化 年月日T时分秒
            std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);

            // 5. 拼接毫秒和 Z，输出到 stderr
            std::cerr << "[DEBUG " << buf << '.' << std::setfill('0') << std::setw(3) << ms.count() << "Z] " << message << std::endl;
        }
    }

    void debug_print(const std::string &message)
    {
        if (util::resolve_value(config::get("701998cfd3d1ff0378375a9826fd9edd")) == "yes")
            std::cerr << message << std::endl;
    }

    void load_config()
    {
#if defined(_WIN32)
        auto system_config_path = std::filesystem::path(getenv("ProgramData")) / "ghcd" / "system.ksv";
        user_config_path = std::filesystem::path(getenv("LOCALAPPDATA")) / "ghcd" / "user.ksv";
        place_local_index = std::filesystem::path(getenv("LOCALAPPDATA")) / "ghcd" / "place.cds";
        auto place_root = std::filesystem::path(getenv("USERPROFILE")) / ".ghcd" / "place";
#else
        auto system_config_path = std::filesystem::path("/etc/ghcd/system.ksv");
        const char *xdg = getenv("XDG_CONFIG_HOME");
        const char *home = getenv("HOME");
        std::filesystem::path config_root;
        if (xdg && *xdg)
            config_root = xdg;
        else
            config_root = std::filesystem::path(home) / ".config";
        user_config_path = config_root / "ghcd" / "user.ksv";
        place_local_index = config_root / "ghcd" / "place.ksv";
        auto place_root = std::filesystem::path(getenv("HOME")) / ".ghcd" / "place";
#endif
        std::ifstream system_config_file(system_config_path);
        std::ifstream user_config_file(user_config_path);
        std::ifstream place_local_index_file(place_local_index);
        std::string system_config_content((std::istreambuf_iterator<char>(system_config_file)), std::istreambuf_iterator<char>());
        user_config_content = std::string((std::istreambuf_iterator<char>(user_config_file)), std::istreambuf_iterator<char>());
        place_local_index_content = std::string((std::istreambuf_iterator<char>(place_local_index_file)), std::istreambuf_iterator<char>());
        config::parse_ksv_string(system_config_content);
        config::parse_ksv_string(user_config_content);
        cds::load(place_cds_node, place_local_index_content);
        if (cds::fetch(place_cds_node, {"place_name_index_list", config::get("bcf53b4b68691525f217aa8150dc6c6b")}) != "")
        {
            place_config_path = place_root / cds::fetch(place_cds_node, {"place_name_index_list", config::get("bcf53b4b68691525f217aa8150dc6c6b")}) / "place.cds";
            std::ifstream place_config_file(place_config_path);
            place_config_content = std::string((std::istreambuf_iterator<char>(place_config_file)), std::istreambuf_iterator<char>());
            config::parse_ksv_string(place_config_content);
        }
    }
}