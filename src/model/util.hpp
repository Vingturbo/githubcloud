#pragma once
#include <string>
#include <filesystem>
#include "cds.hpp"

namespace util
{
    extern cds::Root place_cds_node;
    extern std::filesystem::path user_config_path;
    extern std::filesystem::path place_local_index;
    extern std::filesystem::path place_config_path;
    extern std::string user_config_content;
    extern std::string place_local_index_content;
    extern std::string place_config_content;

    std::string exec_command(const std::string &cmd);
    std::string resolve_value(const std::string &raw_val);
    void message_out(const std::string &message_id);
    void load_online_config(const std::string &online_config_URL);
    void debug_log(const std::string &message);
    void debug_print(const std::string &message);
    void load_config();
}