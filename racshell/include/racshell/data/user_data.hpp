#ifndef USER_DATA_HPP
#define USER_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct UserData
{
    std::string userid;
    std::string name;
    std::string installation_data;
    std::string owner;
    std::string default_group;
    std::string created_date;
    std::string last_access_date;
    std::string last_access_time;
    bool protected_user = false;
    bool special = false;
    bool operations = false;
    bool auditor = false;
    bool read_only_auditor = false;
    bool revoked = false;
    bool password_expired = false;
    bool has_passphrase = false;
    bool has_password = false;
    bool mfa_active = false;
    std::vector<std::string> groups;
    nlohmann::json security;
    nlohmann::json tso;
    nlohmann::json omvs;
    nlohmann::json kerberos;
    nlohmann::json cics;
    nlohmann::json csdata;
};

#endif // USER_DATA_HPP
