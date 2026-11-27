#ifndef USER_DATA_HPP
#define USER_DATA_HPP

#include <optional>
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
    std::string revoke_date;
    bool protected_user = false;
    bool restrict_global_access_checking = false;
    bool special = false;
    bool operations = false;
    bool auditor = false;
    bool read_only_auditor = false;
    bool revoked = false;
    std::optional<bool> contained;
    bool password_expired = false;
    bool has_passphrase = false;
    bool has_password = false;
    bool mfa_active = false;
    std::optional<std::string> passphrase_change_date;
    std::optional<int> passphrase_change_interval;
    std::optional<std::string> password_change_date;
    std::optional<int> password_change_interval;
    std::vector<std::string> groups;
    nlohmann::json tso;
    nlohmann::json omvs;
    nlohmann::json kerberos;
    nlohmann::json cics;
    nlohmann::json csdata;
};

#endif // USER_DATA_HPP
