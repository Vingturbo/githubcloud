#include <iostream>
#include <cstring>
#include <string_view>
#include "model/cli.hpp"
#include "model/config.hpp"
#include "model/init.hpp"

int main(int argc, char *argv[])
{
    std::string_view d_config="1d510a311320bb3fb97abff44530c9d9 @@出现不受到支持的开关";
    config::parse_ksv_string((std::string)d_config);
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
            cli_main(argc, argv);
        }
    }
    else
    {
        std::cout << "desktop mode is not implemented yet." << std::endl;
    }
    return 0;
}