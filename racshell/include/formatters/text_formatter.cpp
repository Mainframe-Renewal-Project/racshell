#include "racshell/output_formatter.hpp"
#include "include/racshell/command_helper.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <string_view>

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

    void append_boolean(std::stringstream &ss, bool value)
    {
        const char *text = value && racshell::color_output_enabled() ? u8"YES \u2713" : value ? "YES" : "NO";
        racshell::print_colored_text(ss,
                                     value ? racshell::terminal_color::yes : racshell::terminal_color::no,
                                     text);
    }

    void append_label(std::stringstream &ss, std::string_view label)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::text, label);
    }

    void append_boolean_field(std::stringstream &ss, std::string_view label, bool value)
    {
        append_label(ss, label);
        append_boolean(ss, value);
        ss << "\n";
    }

    template <typename T>
    void append_styled_value(std::stringstream &ss, const T &value)
    {
        std::stringstream value_stream;
        value_stream << value;
        racshell::print_colored_text(ss, racshell::terminal_color::value, value_stream.str());
    }

    void append_styled_value(std::stringstream &ss, const nlohmann::json &value)
    {
        if (value.is_boolean())
        {
            append_boolean(ss, value.get<bool>());
        }
        else if (value.is_string())
        {
            append_styled_value(ss, value.get<std::string>());
        }
        else
        {
            append_styled_value(ss, value.dump());
        }
    }

    template <typename T>
    void append_field(std::stringstream &ss, std::string_view label, const T &value)
    {
        append_label(ss, label);
        append_styled_value(ss, value);
        ss << "\n";
    }

    std::string_view tree_branch(bool last)
    {
        if (!racshell::color_output_enabled())
            return last ? "\\-- " : "|-- ";
        return last ? "└─ " : "├─ ";
    }

    std::string_view tree_continuation()
    {
        return racshell::color_output_enabled() ? "│  " : "|  ";
    }

    void append_tree_json(std::stringstream &ss, const nlohmann::json &value, const std::string &prefix = {})
    {
        if (value.is_null() || (value.is_structured() && value.empty()))
        {
            ss << prefix << tree_branch(true);
            append_styled_value(ss, "None");
            ss << "\n";
            return;
        }

        if (value.is_object())
        {
            std::size_t index = 0;
            for (const auto &field : value.items())
            {
                const bool last = ++index == value.size();
                ss << prefix << tree_branch(last);
                append_label(ss, format_security_key(field.key()));
                ss << ":";
                if (field.value().is_structured() && !field.value().empty())
                {
                    ss << "\n";
                    append_tree_json(ss, field.value(), prefix + (last ? "   " : std::string(tree_continuation())));
                }
                else
                {
                    ss << " ";
                    if (field.value().is_structured())
                        append_styled_value(ss, "None");
                    else
                        append_styled_value(ss, field.value());
                    ss << "\n";
                }
            }
        }
        else if (value.is_array())
        {
            for (std::size_t index = 0; index < value.size(); ++index)
            {
                const bool last = index + 1 == value.size();
                ss << prefix << tree_branch(last);
                append_label(ss, "Entry " + std::to_string(index + 1));
                ss << ":";
                if (value[index].is_structured() && !value[index].empty())
                {
                    ss << "\n";
                    append_tree_json(ss, value[index], prefix + (last ? "   " : std::string(tree_continuation())));
                }
                else
                {
                    ss << " ";
                    if (value[index].is_structured())
                        append_styled_value(ss, "None");
                    else
                        append_styled_value(ss, value[index]);
                    ss << "\n";
                }
            }
        }
        else
        {
            ss << prefix << tree_branch(true);
            append_styled_value(ss, value);
            ss << "\n";
        }
    }

    void append_comparison_value_text(std::stringstream &ss, const nlohmann::json &value)
    {
        if (value.is_boolean())
        {
            append_boolean(ss, value.get<bool>());
        }
        else
        {
            append_styled_value(ss, racshell::value_to_text(value));
        }
    }

    void append_segment_label(std::stringstream &ss, std::string_view segment)
    {
        ss << "\n";
        racshell::print_colored_text(ss, racshell::terminal_color::cyan, segment);
        ss << ":";
    }

    void append_segment_heading(std::stringstream &ss, std::string_view segment)
    {
        append_segment_label(ss, segment);
        ss << "\n";
    }

    void append_main_heading(std::stringstream &ss, std::string_view heading)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::cyan, heading);
        ss << "\n\n";
    }

    void append_csdata(std::stringstream &ss, const nlohmann::json &csdata)
    {
        if (!csdata.is_null() && !csdata.empty())
        {
            append_segment_heading(ss, "CSDATA");
            append_tree_json(ss, csdata);
        }
    }

    void append_access_list(std::stringstream &ss, const std::vector<AccessEntry> &access_list)
    {
        if (!access_list.empty())
        {
            racshell::print_colored_text(ss, racshell::terminal_color::heading, "Access List:");
            ss << "\n";
            for (std::size_t index = 0; index < access_list.size(); ++index)
            {
                const auto &entry = access_list[index];
                ss << tree_branch(index + 1 == access_list.size());
                append_styled_value(ss, entry.access_type);
                if (!entry.access_id.empty())
                {
                    ss << " (";
                    append_styled_value(ss, entry.access_id);
                    ss << ")";
                }
                ss << "\n";
            }
        }
    }

    void print_user_list(std::stringstream &ss, const std::string &heading, const nlohmann::json &users)
    {
        append_label(ss, heading);
        ss << "\n";
        for (std::size_t index = 0; index < users.size(); ++index)
        {
            const auto &user = users[index];
            ss << tree_branch(index + 1 == users.size());
            append_styled_value(ss, user.value("userid", ""));
            ss << " (";
            append_styled_value(ss, user.value("authority", ""));
            ss << ")\n";
        }
    }

    void print_group_list(std::stringstream &ss, const std::string &heading, const nlohmann::json &groups)
    {
        append_label(ss, heading);
        ss << "\n";
        for (std::size_t index = 0; index < groups.size(); ++index)
        {
            const auto &group = groups[index];
            ss << tree_branch(index + 1 == groups.size());
            append_styled_value(ss, racshell::value_to_text(group));
            ss << "\n";
        }
    }

    
    void append_comparison_value(std::stringstream &ss,
                                 const std::string &left_groupid,
                                 const std::string &right_groupid,
                                 const nlohmann::json &difference,
                                 const std::string &label)
    {
        ss << "  ";
        const bool segment_label = label == "tso" || label == "kerberos" ||
                       label == "cics" || label == "omvs" || label == "csdata";
        racshell::colorize(ss, segment_label ? racshell::terminal_color::heading : racshell::terminal_color::text) << label;
        racshell::reset_color(ss) << ":\n";

        const nlohmann::json left_value = difference.contains("left") ? difference["left"] : nlohmann::json();
        const nlohmann::json right_value = difference.contains("right") ? difference["right"] : nlohmann::json();
        const auto append_side = [&](const std::string &groupid, const nlohmann::json &value, bool left)
        {
            ss << "    " << tree_branch(!left);
            racshell::colorize(ss, racshell::terminal_color::value) << groupid;
            racshell::reset_color(ss) << ":";
            if (value.is_structured() && !value.empty())
            {
                ss << "\n";
                append_tree_json(ss, value, left ? "    " + std::string(tree_continuation()) : "       ");
            }
            else
            {
                ss << " ";
                append_comparison_value_text(ss, value);
                ss << "\n";
            }
        };

        append_side(left_groupid, left_value, true);
        append_side(right_groupid, right_value, false);
    }

} // namespace

std::string TextFormatter::format(const RrsfData &rrsf)
{
    std::stringstream ss;
    append_main_heading(ss, "RRSF configuration");
    const auto &base = rrsf.profile.at("base");
    for (const auto &field : base.items())
    {
        if (!field.value().is_structured())
            append_field(ss, format_security_key(field.key()) + ": ", field.value());
    }
    for (const auto &field : base.items())
    {
        if (!field.value().is_structured() || field.key() == "base:nodes")
            continue;
        append_segment_heading(ss, format_security_key(field.key()));
        append_tree_json(ss, field.value());
    }
    const auto nodes = base.find("base:nodes");
    if (nodes == base.end() || nodes->empty())
    {
        append_segment_heading(ss, "Nodes");
        append_tree_json(ss, nlohmann::json());
    }
    else if (nodes->is_array())
    {
        for (std::size_t index = 0; index < nodes->size(); ++index)
        {
            append_segment_heading(ss, "Node " + std::to_string(index + 1));
            append_tree_json(ss, (*nodes)[index]);
        }
    }
    return ss.str();
}

std::string TextFormatter::format(const RacfOptionsData &options)
{
    std::stringstream ss;
    append_main_heading(ss, "RACF system options");
    const std::array<std::string, 6> headings = {
        "General", "Password policy", "Auditing", "Security labels", "JES", "Classes"};
    std::array<nlohmann::json, 6> sections;
    for (auto &section : sections)
    {
        section = nlohmann::json::object();
    }
    for (const auto &segment : options.profile.items())
    {
        for (const auto &option : segment.value().items())
        {
            const auto &key = option.key();
            std::size_t section = 0;
            if (key.find("_classes") != std::string::npos || key == "base:raclist")
                section = 5;
            else if (key.find("password") != std::string::npos || key.find("passphrase") != std::string::npos || key == "base:revoke_inactive_userids_interval")
                section = 1;
            else if (key.find("audit") != std::string::npos || key.find("base:log_") == 0)
                section = 2;
            else if (key.find("security_label") != std::string::npos || key.find("multi_level_security") != std::string::npos)
                section = 3;
            else if (key.find("base:jes_") == 0)
                section = 4;
            sections[section][key] = option.value();
        }
    }
    for (std::size_t section = 0; section < sections.size(); ++section)
    {
        if (sections[section].empty())
            continue;
        append_segment_heading(ss, headings[section]);
        std::size_t option_index = 0;
        for (const auto &option : sections[section].items())
        {
            const bool last_option = ++option_index == sections[section].size();
            const std::string option_prefix(tree_branch(last_option));
            const std::string child_prefix = last_option ? "   " : std::string(tree_continuation());
            const std::string label = option.key() == "base:raclist" ? "RACLIST classes" : format_security_key(option.key());
            ss << option_prefix;
            append_label(ss, label);
            ss << ":";
            if (section != 5)
            {
                ss << " ";
                append_styled_value(ss, option.value());
                ss << "\n";
                continue;
            }
            std::vector<std::string> classes;
            if (option.value().is_array())
            {
                for (const auto &name : option.value())
                    classes.push_back(racshell::value_to_text(name));
            }
            else if (option.value().is_string())
            {
                std::istringstream names(option.value().get<std::string>());
                for (std::string name; names >> name;)
                    classes.push_back(name);
            }
            else
            {
                ss << " ";
                append_styled_value(ss, option.value());
                ss << "\n";
                continue;
            }
            if (classes.empty())
            {
                ss << " ";
                append_styled_value(ss, "None");
                ss << "\n";
                continue;
            }
            ss << "\n";
            for (std::size_t index = 0; index < classes.size(); ++index)
            {
                ss << child_prefix << tree_branch(index + 1 == classes.size());
                append_styled_value(ss, classes[index]);
                ss << "\n";
            }
        }
    }
    return ss.str();
}

std::string TextFormatter::format(const UserData &user)
{
    std::stringstream ss;

    append_main_heading(ss, "RACF user");
    append_field(ss, "User: ", user.userid);
    append_field(ss, "Name: ", user.name);
    append_field(ss, "Installation data: ", user.installation_data);
    append_field(ss, "Owner: ", user.owner);
    append_field(ss, "Default group: ", user.default_group);
    append_field(ss, "Created: ", user.created_date);
    append_field(ss, "Last access date: ", user.last_access_date);
    append_field(ss, "Last access time: ", user.last_access_time);
    append_boolean_field(ss, "Protected: ", user.protected_user);
    append_boolean_field(ss, "Restricted: ", user.restrict_global_access_checking);
    append_boolean_field(ss, "Special: ", user.special);
    append_boolean_field(ss, "Operations: ", user.operations);
    append_boolean_field(ss, "Auditor: ", user.auditor);
    append_boolean_field(ss, "Read-only auditor: ", user.read_only_auditor);
    append_boolean_field(ss, "Revoked: ", user.revoked);
    if (!user.revoke_date.empty())
    {
        append_field(ss, "Revoke date: ", user.revoke_date);
    }
    if (user.contained.has_value())
    {
        append_boolean_field(ss, "Contained: ", *user.contained);
    }
    append_boolean_field(ss, "Password expired: ", user.password_expired);
    append_boolean_field(ss, "Has passphrase: ", user.has_passphrase);
    append_boolean_field(ss, "Has password: ", user.has_password);
    append_boolean_field(ss, "MFA active: ", user.mfa_active);

    if (user.passphrase_change_date.has_value())
    {
        append_field(ss, "Passphrase change date: ", *user.passphrase_change_date);
    }
    if (user.passphrase_change_interval.has_value())
    {
        append_field(ss, "Passphrase change interval: ", *user.passphrase_change_interval);
    }
    if (user.password_change_date.has_value())
    {
        append_field(ss, "Password change date: ", *user.password_change_date);
    }
    if (user.password_change_interval.has_value())
    {
        append_field(ss, "Password change interval: ", *user.password_change_interval);
    }

    if (!user.groups.empty())
    {
        append_label(ss, "Groups:");
        ss << "\n";
        for (std::size_t index = 0; index < user.groups.size(); ++index)
        {
            ss << tree_branch(index + 1 == user.groups.size());
            append_styled_value(ss, user.groups[index]);
            ss << "\n";
        }
    }

    if (!user.tso.is_null() && !user.tso.empty())
    {
        append_segment_heading(ss, "TSO");
        append_tree_json(ss, user.tso);
    }

    if (!user.omvs.is_null() && !user.omvs.empty())
    {
        append_segment_heading(ss, "OMVS");
        append_tree_json(ss, user.omvs);
    }

    if (!user.kerberos.is_null() && !user.kerberos.empty())
    {
        append_segment_heading(ss, "Kerberos");
        append_tree_json(ss, user.kerberos);
    }

    if (!user.cics.is_null() && !user.cics.empty())
    {
        append_segment_heading(ss, "CICS");
        append_tree_json(ss, user.cics);
    }

    append_csdata(ss, user.csdata);

    return ss.str();
}

std::string TextFormatter::format(const GroupData &group)
{
    std::stringstream ss;

    append_main_heading(ss, "RACF group");
    append_field(ss, "Group: ", group.groupid);
    append_field(ss, "Owner: ", group.owner);
    append_field(ss, "Created: ", group.created_date);
    append_field(ss, "Superior group: ", group.superior_group);
    append_boolean_field(ss, "Universal: ", group.universal);

    if (!group.installation_data.empty())
    {
        append_field(ss, "Installation data: ", group.installation_data);
    }

    if (!group.connected_users.empty())
    {
        append_label(ss, "Users:");
        ss << "\n";
        for (std::size_t index = 0; index < group.connected_users.size(); ++index)
        {
            const auto &connected_user = group.connected_users[index];
            ss << tree_branch(index + 1 == group.connected_users.size());
            append_styled_value(ss, connected_user.userid);
            ss << " (";
            append_styled_value(ss, connected_user.authority);
            ss << ")\n";
        }
    }

    if (!group.omvs.is_null() && !group.omvs.empty())
    {
        append_segment_heading(ss, "OMVS");
        append_tree_json(ss, group.omvs);
    }

    append_csdata(ss, group.csdata);

    return ss.str();
}

std::string TextFormatter::format(const DatasetData &dataset)
{
    std::stringstream ss;

    append_main_heading(ss, "RACF dataset");
    append_field(ss, "Dataset: ", dataset.dataset);
    append_field(ss, "Owner: ", dataset.owner);
    append_field(ss, "UACC: ", dataset.uacc);
    append_boolean_field(ss, "Audited: ", dataset.audited);

    if (dataset.access_count > 0)
    {
        append_field(ss, "Access Count: ", dataset.access_count);
    }

    append_csdata(ss, dataset.csdata);

    append_access_list(ss, dataset.access_list);

    return ss.str();
}

std::string TextFormatter::format(const ResourceData &resource)
{
    std::stringstream ss;

    append_main_heading(ss, "RACF resource");
    append_field(ss, "Resource: ", resource.resource);
    append_field(ss, "Class: ", resource.resource_class);
    append_field(ss, "Owner: ", resource.owner);
    append_field(ss, "UACC: ", resource.uacc);

    if (resource.base.contains("base:create_date"))
    {
        append_field(ss, "Created: ", resource.base["base:create_date"]);
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
        racshell::print_colored_text(ss, racshell::terminal_color::heading, "Left response:");
        ss << "\n";
        ss << (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty()
                   ? comparison.left.response_json.dump(2)
                   : comparison.left.raw_response_json)
           << "\n";
        racshell::print_colored_text(ss, racshell::terminal_color::heading, "Right response:");
        ss << "\n";
        ss << (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty()
                   ? comparison.right.response_json.dump(2)
                   : comparison.right.raw_response_json)
           << "\n";
        return ss.str();
    }

    if (comparison.identical)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::gray,
            "Groups " + comparison.left.groupid + 
            " and " + comparison.right.groupid +
            " are identical for compared fields.\n");
        return ss.str();
    }

    racshell::print_colored_text(ss,
                                 racshell::terminal_color::gray,
                                 "Comparing groups " + comparison.left.groupid + " and " + comparison.right.groupid + ":\n");
    for (auto it = comparison.differences.begin(); it != comparison.differences.end(); ++it)
    {
        if (it.key() == "connected_users")
        {
            const nlohmann::json &users = it.value();
            if (users.contains("only_in_left"))
            {
                racshell::colorize(ss, racshell::terminal_color::heading) << "  Users only in " << comparison.left.groupid << ":\n";
                racshell::reset_color(ss);
                const auto &left_users = users["only_in_left"];
                for (std::size_t index = 0; index < left_users.size(); ++index)
                {
                    const auto &user = left_users[index];
                    ss << "  " << tree_branch(index + 1 == left_users.size());
                    append_styled_value(ss, user.value("userid", ""));
                    ss << " (";
                    append_styled_value(ss, user.value("authority", ""));
                    ss << ")\n";
                }
            }
            if (users.contains("only_in_right"))
            {
                racshell::colorize(ss, racshell::terminal_color::heading) << "  Users only in " << comparison.right.groupid << ":\n";
                racshell::reset_color(ss);
                const auto &right_users = users["only_in_right"];
                for (std::size_t index = 0; index < right_users.size(); ++index)
                {
                    const auto &user = right_users[index];
                    ss << "  " << tree_branch(index + 1 == right_users.size());
                    append_styled_value(ss, user.value("userid", ""));
                    ss << " (";
                    append_styled_value(ss, user.value("authority", ""));
                    ss << ")\n";
                }
            }
            if (users.contains("authority_mismatches"))
            {
                racshell::print_colored_text(ss, racshell::terminal_color::gray, "  User authority differences\n");
                const auto &mismatches = users["authority_mismatches"];
                for (std::size_t index = 0; index < mismatches.size(); ++index)
                {
                    const auto &mismatch = mismatches[index];
                    const bool last = index + 1 == mismatches.size();
                    ss << "  " << tree_branch(last);
                    append_styled_value(ss, mismatch.value("userid", ""));
                    ss << ":\n" << (last ? "     " : "  " + std::string(tree_continuation())) << tree_branch(false);
                    racshell::colorize(ss, racshell::terminal_color::value) << comparison.left.groupid;
                    racshell::reset_color(ss) << ": ";
                    append_styled_value(ss, mismatch.value("left", ""));
                    ss << "\n" << (last ? "     " : "  " + std::string(tree_continuation())) << tree_branch(true);
                    racshell::colorize(ss, racshell::terminal_color::value) << comparison.right.groupid;
                    racshell::reset_color(ss) << ": ";
                    append_styled_value(ss, mismatch.value("right", ""));
                    ss << "\n";
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
        racshell::print_colored_text(ss, racshell::terminal_color::heading, "Left response:");
        ss << "\n";
        ss << (!comparison.left.response_json.is_null() && !comparison.left.response_json.empty()
                   ? comparison.left.response_json.dump(2)
                   : comparison.left.raw_response_json)
           << "\n";
        racshell::print_colored_text(ss, racshell::terminal_color::heading, "Right response:");
        ss << "\n";
        ss << (!comparison.right.response_json.is_null() && !comparison.right.response_json.empty()
                   ? comparison.right.response_json.dump(2)
                   : comparison.right.raw_response_json)
           << "\n";
        return ss.str();
    }

    if (comparison.identical)
    {
        racshell::print_colored_text(ss, racshell::terminal_color::gray,
            "Users " + comparison.left.userid + 
            " and " + comparison.right.userid +
            " are identical for compared fields.\n");
        return ss.str();
    }

    racshell::print_colored_text(ss,
                                 racshell::terminal_color::gray,
                                 "Comparing users " + comparison.left.userid + " and " + comparison.right.userid + ":\n");

    const std::array<const char *, 12> preferred_order = {
        "name",
        "owner",
        "default_group",
        "created_date",
        "revoked",
        "security",
        "tso",
        "kerberos",
        "cics",
        "groups",
        "omvs",
        "csdata"};

    const auto append_difference_by_key = [&](const std::string &key)
    {
        if (!comparison.differences.contains(key))
        {
            return;
        }

        const nlohmann::json &difference = comparison.differences[key];
        if (key == "groups")
        {
            racshell::colorize(ss, racshell::terminal_color::gray) << "  " << key;
            racshell::reset_color(ss) << ": \n";
            const nlohmann::json &groups = difference;
            if (groups.contains("only_in_left") && groups["only_in_left"].is_array() && !groups["only_in_left"].empty())
            {
                racshell::colorize(ss, racshell::terminal_color::heading) << "    Groups only in " << comparison.left.userid << ":\n";
                racshell::reset_color(ss);
                const auto &left_groups = groups["only_in_left"];
                for (std::size_t index = 0; index < left_groups.size(); ++index)
                {
                    ss << "    " << tree_branch(index + 1 == left_groups.size());
                    append_styled_value(ss, racshell::value_to_text(left_groups[index]));
                    ss << "\n";
                }
            }
            if (groups.contains("only_in_right") && groups["only_in_right"].is_array() && !groups["only_in_right"].empty())
            {
                racshell::colorize(ss, racshell::terminal_color::heading) << "    Groups only in " << comparison.right.userid << ":\n";
                racshell::reset_color(ss);
                const auto &right_groups = groups["only_in_right"];
                for (std::size_t index = 0; index < right_groups.size(); ++index)
                {
                    ss << "    " << tree_branch(index + 1 == right_groups.size());
                    append_styled_value(ss, racshell::value_to_text(right_groups[index]));
                    ss << "\n";
                }
            }
            return;
        }

        append_comparison_value(ss,
                                comparison.left.userid,
                                comparison.right.userid,
                                difference,
                                key);
    };

    for (const char *key : preferred_order)
    {
        append_difference_by_key(key);
    }

    for (auto it = comparison.differences.begin(); it != comparison.differences.end(); ++it)
    {
        if (std::find(preferred_order.begin(), preferred_order.end(), it.key()) != preferred_order.end())
        {
            continue;
        }

        append_difference_by_key(it.key());
    }

    return ss.str();
}
