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
        {"P", "place"},
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
        {"r", "restore"},
        {"a", "add"},
        {"w", "use"},
        {"x", "remove"},
        // 修饰开关
        {"o", "out"},
        {"p", "password"},
        {"n", "name"},
        {"k", "key"},
        {"b", "base64"},
        {"t", "timestamp"},
        // 即时开关
        {"i", "include-online-config"},
        {"j", "jump-config"}};
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
#define _MAIN_OPT_ARG "47c99378cc29eb250bd790fc0bc74faa"

    std::vector<OptionSpec> OptDef = {
        // ================= 开关类 (注册 _OPT_CLASS) =================
        // 类必须包含所有它允许的主开关，以及主开关衍生出来的修饰符
        {"internet", {"internet", "upload", "download", "search", "out", "password", "name", "base64"}, _OPT_CLASS, "", ""},
        {"local", {"local", "search", "view", "restore", "base64", "key", "out", "timestamp"}, _OPT_CLASS, "", ""},
        {"config", {"config", "my-config-set", "global-config-set", "view"}, _OPT_CLASS, "", ""},
        {"share", {"share", "create", "fetch", "password", "out"}, _OPT_CLASS, "", ""},
        {"place", {"place", "add", "use", "remove"}, _OPT_CLASS, "", ""},
        {"other", {"other", "fetch", "out"}, _OPT_CLASS, "", ""},

        // ================= 主开关 (注册 _MAIN_OPT) =================
        // 主开关必须包含自己的类、自己的修饰符、以及自己的从属的开关类
        {"version", {"version"}, _MAIN_OPT, "", ""},
        {"help", {"help"}, _MAIN_OPT, "", ""},
        {"upload", {"upload", "internet", "password", "name", "base64"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"download", {"download", "internet", "password", "out"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"search", {"search", "internet", "local"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"view", {"view", "local", "config"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"create", {"create", "share", "password"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"fetch", {"fetch", "share", "other", "out"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"my-config-set", {"my-config-set", "config"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"global-config-set", {"global-config-set", "config"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"restore", {"restore", "local", "base64", "key", "timestamp", "out"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"add", {"place", "add"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"use", {"place", "use"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},
        {"remove", {"place", "remove"}, _MAIN_OPT, _MAIN_OPT_ARG, ""},

        // ================= 修饰符 (消费参数) =================
        // 修饰符必须包含：自己、它允许伴随的主开关、该主开关下的操作类、以及其他可共存的修饰符
        {"out", {"out", "download", "fetch", "restore", "internet", "password", "base64", "local", "key", "timestamp", "share", "other"}, "", "e1d357e4802e1c961acc37482dcc5e10", ""},
        {"password", {"password", "upload", "download", "create", "internet", "base64", "out", "share"}, "", "bdc0bfa3088d5feedc1588c7807f05ec", ""},
        {"name", {"name", "upload", "internet", "base64"}, "", "d2e2560089221ad15d2ec113e2f5b599", ""},
        {"key", {"key", "restore", "local", "timestamp", "base64", "out"}, "", "be3a7f9bfab0c26117cb25935df0fc8c", ""},
        {"timestamp", {"timestamp", "restore", "local", "key", "base64", "out"}, "", "0c6482892e9df6505c8de1eabca68266", ""},

        // ================= 无参修饰符 (传入 ebl_cfg) =================
        {"base64", {"base64", "upload", "restore", "internet", "password", "name", "out", "local", "key", "timestamp"}, "", "", "e3656dd18b1456b33344ed264a27bf5c @@yes"}};
    /*
    #undef _OPT_CLASS
    #undef _MAIN_OPT
    #undef _MAIN_OPT_ARG
    */

    std::vector<std::string> options_have;
    int argeat = 0;

    bool _add_opt(std::string opt_text)
    {
        // 1. 查表：寻找该开关的定义
        const OptionSpec *spec = nullptr;
        for (const auto &s : OptDef)
        {
            if (s.name == opt_text)
            {
                spec = &s;
                break;
            }
        }

        // 不存在的开关
        if (!spec)
        {
            util::message_out("4ce3b19939d8cec5e725f8696a45e663");
            return true;
        }

        // 2. 兼容性无向图校验（求交集思想）
        // 遍历之前已经成功添加的开关，看看新开关在不在它们的兼容列表里
        for (const auto &existing_opt : options_have)
        {
            bool found = false;
            for (const auto &s : OptDef)
            {
                if (s.name == existing_opt)
                {
                    // 在已存在开关的 compatible 列表中寻找新开关
                    for (const auto &c : s.compatible)
                    {
                        if (c == opt_text)
                        {
                            found = true;
                            break;
                        }
                    }
                    break;
                }
            }

            // 只要有一个已存在的开关不兼容它，立刻报错
            if (!found)
            {
                util::message_out("4ce3b19939d8cec5e725f8696a45e663");
                return true;
            }
        }

        // 3. 核心冲突检测（利用你天才般的 reg_key 逻辑）
        if (!spec->reg_key.empty())
        {
            // 如果该 key 不存在，返回 opt_text；如果存在，返回之前存入的值。
            // 如果之前的操作是另一个同类（比如之前是 upload，现在是 download），两者不等，触发冲突！
            std::string existing = config::get(spec->reg_key, "@@" + opt_text);
            if (util::resolve_value(existing) != opt_text)
            {
                util::message_out("4ce3b19939d8cec5e725f8696a45e663");
                return true;
            }
            // 注册进配置
            config::set(spec->reg_key, "@@" + opt_text);
        }

        // 4. 参数消费
        if (!spec->opt.empty())
        {
            // 如果参数不够用了，报错
            if (argeat >= arguments.size())
            {
                util::message_out("eddeb0cca6c84cffa12b5a7fb2900055");
                return true;
            }
            // 存入参数，并把游标后移
            config::set(spec->opt, arguments[argeat]);
            argeat++;
        }

        // 5. 无参修饰符的特殊行为（比如 base64, complete）
        if (!spec->ebl_cfg.empty())
        {
            config::parse_ksv_string(spec->ebl_cfg);
        }

        // 6. 成功添加，记录到已出现列表
        options_have.push_back(opt_text);

        return false;
    }

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
                if(arg.substr(0,2)=="@!")util::resolve_value(arg);
                arguments.push_back(arg);
            }
        }

        // 开关解析与参数填充
        for (std::string i : options)
        {
            if (i == "jump-config")
            {
                config::parse_ksv_string(util::resolve_value(arguments[argeat]));
                argeat++;
                continue;
            }
            if (i == "include-online-config")
            {
                util::load_online_config(util::resolve_value(arguments[argeat]));
                argeat++;
                continue;
            }
            if (i == "debug")
            {
                config::set("701998cfd3d1ff0378375a9826fd9edd", "@@yes");
                continue;
            }
            if(i=="complete"){
                config::set("c28e2e867367b3ff652b3f0bd986c4fb", "@@yes");
                continue;
            }
            if (_add_opt(i))
                return;
        }
        if (argeat != arguments.size())
        {
            util::message_out("eddeb0cca6c84cffa12b5a7fb2900055");
            return;
        }

        //开关和参数处理
        if(util::resolve_value(config::get("96d12637105e2b0f518e06ebce6fbf85"))=="help"){
            std::cerr<<init::help_doc<<std::endl;
        }
        if(util::resolve_value(config::get("96d12637105e2b0f518e06ebce6fbf85"))=="version"){
            std::cerr<<"GHCD_VERSION: "<<_GHCD_VERSION<<std::endl;
            std::cerr<<"MODEL_VERSION: "<<_MODEL_VERSION<<std::endl;
            std::cerr<<"Copyright © 2026 Vingturbo"<<std::endl;
            std::cerr<<"SPDX-License-Identifier: AGPL-3.0-or-later"<<std::endl;
        }
    }
}