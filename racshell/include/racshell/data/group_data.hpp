#ifndef GROUP_DATA_HPP
#define GROUP_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct GroupUser
{
    std::string userid;
    std::string authority;
};

struct GroupData
{
    std::string groupid;
    std::string owner;
    std::string created_date;
    std::string superior_group;
    std::string installation_data;
    bool universal = false;
    bool terminal_universal_access = false;
    std::vector<GroupUser> connected_users;
    nlohmann::json omvs;
    nlohmann::json csdata; 
};

#endif // GROUP_DATA_HPP
