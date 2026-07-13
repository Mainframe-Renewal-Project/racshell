#include "racshell/output_formatter.hpp"
#include "include/racshell/command_helper.hpp"
#include <cctype>
#include <sstream>

namespace
{

    std::string format_security_key(std::string key)
    {
        const std::string::size_type separator_pos = key.find_last_of(':');
        if (separator_pos != std::string::npos)
        {
            key = key.substr(separator_pos + 1);
        }

        for (char &c : key)
        {
            if (c == '_')
            {
                c = ' ';
            }
        }

        if (!key.empty())
        {
            key[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(key[0])));
        }

        return key;
    }

    void append_csdata(std::stringstream &ss, const nlohmann::json &csdata)
    {
        if (!csdata.is_null() && !csdata.empty())
        {
            ss << "CSDATA:\n";
            for (auto it = csdata.begin(); it != csdata.end(); ++it)
            {
                ss << "  " << format_security_key(it.key()) << ": ";
                if (it.value().is_string())
                {
                    ss << it.value().get<std::string>() << "\n";
                }
                else
                {
                    ss << it.value().dump() << "\n";
                }
            }
        }
    }

    void append_access_list(std::stringstream &ss, const std::vector<AccessEntry> &access_list)
    {
        if (!access_list.empty())
        {
            ss << "Access List:\n";
            for (const auto &entry : access_list)
            {
                ss << "  " << entry.access_type;
                if (!entry.access_id.empty())
                {
                    ss << " (" << entry.access_id << ")";
                }
                ss << "\n";
            }
        }
    }

    void print_user_list(std::stringstream &ss, const std::string &heading, const nlohmann::json &users)
    {
        ss << heading << "\n";
        for (const auto &user : users)
        {
            ss << "    " << user.value("userid", "")
               << " (" << user.value("authority", "") << ")\n";
        }
    }

    void print_group_list(std::stringstream &ss, const std::string &heading, const nlohmann::json &groups)
    {
        ss << heading << "\n";
        for (const auto &group : groups)
        {
            ss << "    " << racshell::value_to_text(group) << "\n";
        }
    }

    
    void append_comparison_value(std::stringstream &ss,
                                 const std::string &left_groupid,
                                 const std::string &right_groupid,
                                 const nlohmann::json &difference,
                                 const std::string &label)
    {
        ss << "  ";
        racshell::colorize(ss, racshell::terminal_color::blue) << label;
        racshell::reset_color(ss) << ": ";

        racshell::colorize(ss, racshell::terminal_color::magenta) << left_groupid;
        racshell::reset_color(ss) << "='" << racshell::value_to_text(difference.contains("left") ? difference["left"] : nlohmann::json()) << "', ";

        racshell::colorize(ss, racshell::terminal_color::cyan) << right_groupid;
        racshell::reset_color(ss) << "='" << racshell::value_to_text(difference.contains("right") ? difference["right"] : nlohmann::json()) << "'\n";
    }

} // namespace

std::string TextFormatter::format(const UserData &user)
{
    std::stringstream ss;

    ss << "User: " << user.userid << "\n";
    ss << "Name: " << user.name << "\n";
    ss << "Owner: " << user.owner << "\n";
    ss << "Created: " << user.created_date << "\n";
    ss << "Revoked: " << (user.revoked ? "YES" : "NO") << "\n";

    if (!user.groups.empty())
    {
        ss << "Groups:\n";
        for (const auto &group : user.groups)
        {
            ss << "  " << group << "\n";
        }
    }

    if (!user.security.is_null() && !user.security.empty())
    {
        ss << "Security:\n";
        for (auto it = user.security.begin(); it != user.security.end(); ++it)
        {
            ss << "  " << format_security_key(it.key()) << ": ";
            if (it.value().is_string())
            {
                ss << it.value().get<std::string>() << "\n";
            }
            else
            {
                ss << it.value().dump() << "\n";
            }
        }
    }

    if (!user.tso.is_null() && !user.tso.empty())
    {
        ss << "TSO:\n";
        if (user.tso.contains("tso:logon_procedure"))
            ss << "  Procedure:    " << user.tso["tso:logon_procedure"].get<std::string>() << "\n";
        if (user.tso.contains("tso:account_number"))
            ss << "  Account:      " << user.tso["tso:account_number"].get<std::string>() << "\n";
        if (user.tso.contains("tso:default_region_size"))
            ss << "  Region:       " << user.tso["tso:default_region_size"] << "\n";
        if (user.tso.contains("tso:max_region_size"))
            ss << "  Max region:   " << user.tso["tso:max_region_size"] << "\n";
        if (user.tso.contains("tso:dataset_allocation_unit"))
            ss << "  Alloc unit:   " << user.tso["tso:dataset_allocation_unit"].get<std::string>() << "\n";
        if (user.tso.contains("tso:logon_command"))
            ss << "  Logon cmd:    " << user.tso["tso:logon_command"].get<std::string>() << "\n";
        if (user.tso.contains("tso:user_data"))
            ss << "  User data:    " << user.tso["tso:user_data"].get<std::string>() << "\n";
    }

    if (!user.omvs.is_null() && !user.omvs.empty())
    {
        ss << "OMVS:\n";
        if (user.omvs.contains("omvs:uid"))
            ss << "  UID:   " << user.omvs["omvs:uid"] << "\n";
        if (user.omvs.contains("omvs:home_directory"))
            ss << "  Home:  " << user.omvs["omvs:home_directory"].get<std::string>() << "\n";
        if (user.omvs.contains("omvs:default_shell"))
            ss << "  Shell: " << user.omvs["omvs:default_shell"].get<std::string>() << "\n";
    }

    if (!user.kerberos.is_null() && !user.kerberos.empty())
    {
        ss << "Kerberos: " << user.kerberos.dump() << "\n";
    }

    if (!user.cics.is_null() && !user.cics.empty())
    {
        ss << "CICS: " << user.cics.dump() << "\n";
    }

    append_csdata(ss, user.csdata);

    return ss.str();
}

std::string TextFormatter::format(const GroupData &group)
{
    std::stringstream ss;

    ss << "Group: " << group.groupid << "\n";
    ss << "Owner: " << group.owner << "\n";
    ss << "Created: " << group.created_date << "\n";
    ss << "Superior group: " << group.superior_group << "\n";
    ss << "Universal: " << (group.universal ? "YES" : "NO") << "\n";

    if (!group.installation_data.empty())
    {
        ss << "Installation data: " << group.installation_data << "\n";
    }

    if (!group.connected_users.empty())
    {
        ss << "Users:\n";
        for (const auto &connected_user : group.connected_users)
        {
            ss << "  " << connected_user.userid << " ("
               << connected_user.authority << ")\n";
        }
    }

    if (!group.omvs.is_null() && !group.omvs.empty())
    {
        ss << "OMVS:\n";
        if (group.omvs.contains("omvs:gid"))
        {
            ss << "  GID: " << group.omvs["omvs:gid"] << "\n";
        }
    }

    append_csdata(ss, group.csdata);

    return ss.str();
}

std::string TextFormatter::format(const DatasetData &dataset)
{
    std::stringstream ss;

    ss << "Dataset: " << dataset.dataset << "\n";
    ss << "Owner: " << dataset.owner << "\n";
    ss << "UACC: " << dataset.uacc << "\n";
    ss << "Audited: " << (dataset.audited ? "YES" : "NO") << "\n";

    if (dataset.access_count > 0)
    {
        ss << "Access Count: " << dataset.access_count << "\n";
    }

    append_csdata(ss, dataset.csdata);

    append_access_list(ss, dataset.access_list);

    return ss.str();
}

std::string TextFormatter::format(const ResourceData &resource)
{
    std::stringstream ss;

    ss << "Resource: " << resource.resource << "\n";
    ss << "Class: " << resource.resource_class << "\n";
    ss << "Owner: " << resource.owner << "\n";
    ss << "UACC: " << resource.uacc << "\n";

    if (resource.base.contains("base:create_date"))
    {
        ss << "Created: " << resource.base["base:create_date"] << "\n";
    }

    append_csdata(ss, resource.csdata);

    append_access_list(ss, resource.access_list);

    return ss.str();
}

std::string TextFormatter::format(const GroupComparisonData &comparison)
{
    std::stringstream ss;

    if (comparison.raw_json_output)
    {
        ss << "Left response:\n";
        ss << (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty()
                   ? comparison.left.response_json.dump(2)
                   : comparison.left.raw_response_json)
           << "\n";
        ss << "Right response:\n";
        ss << (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty()
                   ? comparison.right.response_json.dump(2)
                   : comparison.right.raw_response_json)
           << "\n";
        return ss.str();
    }

    if (comparison.identical)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::blue,
            "Groups " + comparison.left.groupid + 
            " and " + comparison.right.groupid +
            " are identical for compared fields.\n");
        return ss.str();
    }

    racshell::print_colored_text(ss,
                                 racshell::terminal_color::blue,
                                 "Comparing groups " + comparison.left.groupid + " and " + comparison.right.groupid + ":\n");
    for (auto it = comparison.differences.begin(); it != comparison.differences.end(); ++it)
    {
        if (it.key() == "connected_users")
        {
            const nlohmann::json &users = it.value();
            if (users.contains("only_in_left"))
            {
                racshell::colorize(ss, racshell::terminal_color::magenta) << "  Users only in " << comparison.left.groupid << ":\n";
                racshell::reset_color(ss);
                for (const auto &user : users["only_in_left"])
                {
                    ss << "    " << user.value("userid", "")
                       << " (" << user.value("authority", "") << ")\n";
                }
            }
            if (users.contains("only_in_right"))
            {
                racshell::colorize(ss, racshell::terminal_color::cyan) << "  Users only in " << comparison.right.groupid << ":\n";
                racshell::reset_color(ss);
                for (const auto &user : users["only_in_right"])
                {
                    ss << "    " << user.value("userid", "")
                       << " (" << user.value("authority", "") << ")\n";
                }
            }
            if (users.contains("authority_mismatches"))
            {
                racshell::print_colored_text(ss, racshell::terminal_color::blue, "  User authority differences\n");
                for (const auto &mismatch : users["authority_mismatches"])
                {
                    ss << "    " << mismatch.value("userid", "") << ": ";
                    racshell::colorize(ss, racshell::terminal_color::magenta) << comparison.left.groupid;
                    racshell::reset_color(ss) << "='" << mismatch.value("left", "") << "', ";
                    racshell::colorize(ss, racshell::terminal_color::cyan) << comparison.right.groupid;
                    racshell::reset_color(ss) << "='" << mismatch.value("right", "") << "'\n";
                }
            }
            continue;
        }

        append_comparison_value(ss,
                                comparison.left.groupid,
                                comparison.right.groupid,
                                it.value(),
                                it.key());
    }

    return ss.str();
}

std::string TextFormatter::format(const UserComparisonData &comparison)
{
    std::stringstream ss;

    if (comparison.raw_json_output)
    {
        ss << "Left response:\n";
        ss << (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty()
                   ? comparison.left.response_json.dump(2)
                   : comparison.left.raw_response_json)
           << "\n";
        ss << "Right response:\n";
        ss << (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty()
                   ? comparison.right.response_json.dump(2)
                   : comparison.right.raw_response_json)
           << "\n";
        return ss.str();
    }

    if (comparison.identical)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::blue,
            "Users " + comparison.left.userid + 
            " and " + comparison.right.userid +
            " are identical for compared fields.\n");
        return ss.str();
    }

    racshell::print_colored_text(ss,
                                 racshell::terminal_color::blue,
                                 "Comparing users " + comparison.left.userid + " and " + comparison.right.userid + ":\n");
    for (auto it = comparison.differences.begin(); it != comparison.differences.end(); ++it)
    {
        if (it.key() == "groups")
        {
            const nlohmann::json &groups = it.value();
            if (groups.contains("only_in_left") && groups["only_in_left"].is_array() && !groups["only_in_left"].empty())
            {
                racshell::colorize(ss, racshell::terminal_color::magenta) << "  Groups only in " << comparison.left.userid << ":\n";
                racshell::reset_color(ss);
                for (const auto &group : groups["only_in_left"])
                {
                    ss << "    " << racshell::value_to_text(group) << "\n";
                }
            }
            if (groups.contains("only_in_right") && groups["only_in_right"].is_array() && !groups["only_in_right"].empty())
            {
                racshell::colorize(ss, racshell::terminal_color::cyan) << "  Groups only in " << comparison.right.userid << ":\n";
                racshell::reset_color(ss);
                for (const auto &group : groups["only_in_right"])
                {
                    ss << "    " << racshell::value_to_text(group) << "\n";
                }
            }
            continue;
        }

        append_comparison_value(ss,
                                comparison.left.userid,
                                comparison.right.userid,
                                it.value(),
                                it.key());
    }

    return ss.str();
}
