#include "util.hpp"
#include <string>
#include <map>
#include <cstdio>
#include <iostream>
#include "config.hpp"

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
        // 1. 如果长度小于2，或者不以 @ 开头，说明是不合规的垃圾，直接原样返回（或者你想让它崩就崩）
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
        else if (raw_val[1] == '_')
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

        // 3. 如果写了单 @ 之类的奇怪东西，原样返回，让它去后面报错
        return "";
    }

    void message_out(const std::string &message_id)
    {
        std::cerr << util::resolve_value(config::get(message_id)) << std::endl;
    }
}