#pragma once
#include <string>

namespace util
{
    std::string exec_command(const std::string &cmd);
    std::string resolve_value(const std::string &raw_val);
}