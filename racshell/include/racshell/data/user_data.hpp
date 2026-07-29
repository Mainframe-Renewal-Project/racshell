#ifndef USER_DATA_HPP
#define USER_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct UserData
{
    std::string userid;
    std::string name;
    std::string owner;
    std::string created_date;
    bool revoked = false;
    std::vector<std::string> groups;
    nlohmann::json security;
    nlohmann::json tso;
    nlohmann::json omvs;
    nlohmann::json kerberos;
    nlohmann::json cics;
    nlohmann::json csdata;
};

#endif // USER_DATA_HPP
