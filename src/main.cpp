#include <iostream>
#include <cstring>
#include "model/cli.hpp"

int main(int argc, char *argv[])
{
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