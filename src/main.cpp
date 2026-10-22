#include <iostream>
#include <cstring>
#include <string_view>
#include "model/cli.hpp"
#include "model/config.hpp"
#include "model/init.hpp"

int main(int argc, char *argv[])
{
    // i18n默认文本
    config::parse_ksv_string(R"(
1d510a311320bb3fb97abff44530c9d9 @@出现不受到支持的开关
8a94c5bae8543a47fb0b5dcdc86240e4 @@警告: 在没有开启高级参数的情况下，仍然出现了不是字面量的参数值
)");
    // 默认配置参数
    config::parse_ksv_string(R"(
bdc0bfa3088d5feedc1588c7807f05ec @@please_change_this_password
e1d357e4802e1c961acc37482dcc5e10 @@./
)");
    if ((argv[argc - 1][0] == '-') || (argv[argc - 1][0] == '@'))
    {
        if (strcmp(argv[argc - 1], "--web") == 0)
        {
            std::cout << "Web mode is not implemented yet." << std::endl;
        }
        else if (strcmp(argv[argc - 1], "--desktop") == 0)
        {
            std::cout << "desktop mode is not implemented yet." << std::endl;
        }
        else
        {
            cli::cli_main(argc, argv);
        }
    }
    else
    {
        std::cout << "desktop mode is not implemented yet." << std::endl;
    }
    return 0;
}