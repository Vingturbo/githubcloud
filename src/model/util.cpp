#include "util.hpp"
#include "config.hpp"
#include <string>
#include <map>
#include <cstdio>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

// 如果 Windows 下需要
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace util
{
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
            std::cerr << "[DEBUG " << buf << '.'<< std::setfill('0') << std::setw(3) << ms.count()<< "Z] " << message << std::endl;
        }
    }
}