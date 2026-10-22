#include "init.hpp"
#include "cli.hpp"
#include "config.hpp"
#include "util.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string_view>

namespace cli
{
    std::map<std::string, std::string> options_label = {
        // 开关类
        {"I", "internet"},
        {"L", "local"},
        {"C", "config"},
        {"S", "share"},
        {"O", "other"},
        {"V", "version"},
        // 主开关
        {"u", "upload"},
        {"d", "download"},
        {"s", "search"},
        {"v", "view"},
        {"c", "create"},
        {"f", "fetch"},
        {"h", "help"},
        {"m", "my-config-set"},
        {"g", "global-config-set"},
        // 修饰开关
        {"o", "out"},
        {"p", "password"},
        {"n", "name"},
        // 即时开关
        {"i", "include-online-config"},
        {"j", "jump_config"}};
    std::vector<std::string> options;
    std::vector<std::string> arguments;

    struct OptionSpec
    {
        std::string name;                    // 长名字
        std::vector<std::string> compatible; // 兼容列表
        std::string reg_key;                 // 冲突检测：存入哪个 Key 以检查是否重复注册
        std::string opt;                     // 参数消费：需要参数时，把参数存到这个 Key
        std::string ebl_cfg;                 // 特殊行为：触发时，存入这个固定值
    };

#define _OPT_CLASS "751d01cb8d7a2fc8a2884fd97530a127"
#define _MAIN_OPT "96d12637105e2b0f518e06ebce6fbf85"
#define _MAIN_OPT_AGE "47c99378cc29eb250bd790fc0bc74faa"
    std::vector<OptionSpec> OptDef = {
        // ================= 开关类 (注册 _OPT_CLASS) =================
        {"internet", {"internet", "upload", "download", "search", "out", "password", "name", "complete"}, _OPT_CLASS, "", ""},
        {"local", {"local", "search", "view"}, _OPT_CLASS, "", ""},
        {"config", {"config", "my-config-set", "global-config-set", "view"}, _OPT_CLASS, "", ""},
        {"share", {"share", "create", "fetch", "password", "out"}, _OPT_CLASS, "", ""},
        {"other", {"other", "fetch"}, _OPT_CLASS, "", ""},

        // ================= 主开关 (注册 _MAIN_OPT) =================
        {"version", {"version"}, _MAIN_OPT, "", ""},
        {"help", {"help"}, _MAIN_OPT, "", ""},
        {"upload", {"upload", "internet", "password", "name", "complete"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"download", {"download", "internet", "password", "out", "complete"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"search", {"search", "internet", "local", "complete"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"view", {"view", "local", "config"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"create", {"create", "share", "password"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"fetch", {"fetch", "share", "other", "out"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"my-config-set", {"my-config-set", "config"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},
        {"global-config-set", {"global-config-set", "config"}, _MAIN_OPT, _MAIN_OPT_AGE, ""},

        // ================= 修饰符 (消费参数) =================
        {"out", {"download", "fetch"}, "", "e1d357e4802e1c961acc37482dcc5e10", ""},
        {"password", {"upload", "download", "create"}, "", "bdc0bfa3088d5feedc1588c7807f05ec", ""},
        {"name", {"upload"}, "", "d2e2560089221ad15d2ec113e2f5b599", ""},

        // ================= 无参修饰符 (传入 ebl_cfg 给 parse_ksv_string) =================
        // 注意：这里的字符串必须是 "32位MD5 + 1个空格 + @@yes" 的严格格式！
        {"complete", {"upload", "download", "search"}, "", "", "c28e2e867367b3ff652b3f0bd986c4fb @@yes"}};
    /*
    #undef _OPT_CLASS
    #undef _MAIN_OPT
    #undef _MAIN_OPT_AGE
    */

    void cli_main(int argc, char *argv[])
    {
        // 第一段：从右向左扫描，只为了找到“真正的起点”
        int start_idx = argc;
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
                        if (options_label.count(std::string(1, arg[j])) == 0)
                        {
                            util::message_out("1d510a311320bb3fb97abff44530c9d9");
                            return;
                        }
                        options.push_back(options_label[std::string(1, arg[j])]);
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
}