#pragma once
#include <string>

namespace util
{
    std::string exec_command(const std::string &cmd);
    std::string resolve_value(const std::string &raw_val);
    void message_out(const std::string &message_id);
    void load_online_config(const std::string &online_config_URL);
}