#include "cli.hpp"
#include "config.hpp"
#include "util.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>

std::map<std::string, std::string> options_label;
std::vector<std::string> options;
std::vector<std::string> arguments;

void cli_main(int argc, char *argv[])
{
    // 第一段：从右向左扫描，只为了找到“真正的起点”
    int start_idx = 1;
    for (int i = argc - 1; i >= 1; i--)
    {
        char *arg = argv[i];
        if (arg[0] == '-' || arg[0] == '@')
        {
            start_idx = i; // 更新起点，一直向左找
        }
        else
        {
            break; // 遇到污染，停止
        }
    }

    // 第二段：从左向右扫描，正式开始切分数据（顺序绝对完美！）
    for (int i = start_idx; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg[0] == '-')
        {
            if (arg[1] == '-')
            {
                // 长开关 --web -> web
                options.push_back(arg.substr(2));
            }
            else
            {
                // 短开关组合 -Ido -> I, d, o
                for (size_t j = 1; j < arg.size(); j++)
                {
                    options.push_back(std::string(1, arg[j]));
                }
            }
        }
        else if (arg[0] == '@')
        {
            // 参数：你这里存了带 @ 的，记得后面去前缀
            arguments.push_back(arg);
        }
    }
}